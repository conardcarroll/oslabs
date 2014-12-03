/*
 * Ram backed block device driver.
 *
 * Copyright (C) 2007 Nick Piggin
 * Copyright (C) 2007 Novell Inc.
 *
 * Parts derived from drivers/block/rd.c, and drivers/block/loop.c, copyright
 * of their respective owners.
 *
 * J. Franco - slight modifications for class demonstrations - Oct. 2013
 *             some things may now be broken
 *
 * Focus: radix_tree structure - including operations
 *        block request mechanism
 */
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/major.h>
#include <linux/blkdev.h>
#include <linux/bio.h>              /* block I/O model */
#include <linux/highmem.h>
#include <linux/mutex.h>
#include <linux/radix-tree.h>
#include <linux/fs.h>
#include <linux/slab.h>

#include <asm/uaccess.h>

#define SECTOR_SHIFT		9
#define PAGE_SECTORS_SHIFT	(PAGE_SHIFT - SECTOR_SHIFT)
#define PAGE_SECTORS		(1 << PAGE_SECTORS_SHIFT)

/* Each block ramdisk device has a radix_tree brd_pages of pages that stores
 * the pages containing the block device's contents. A brd page's ->index is
 * its offset in PAGE_SIZE units. 
 *
 * Every block device has a request queue. This is because actual transfers 
 * to and from a disk can take place far away from the time the kernel 
 * requests them, and because the kernel needs the flexibility to schedule 
 * each transfer at the most propitious moment (grouping together, for 
 * instance, requests that affect sectors close together on the disk).  
 * The request function is associated with a request queue when that 
 * queue is created with 'blk_queue_make_request' below.
 *
 * gendisk describes a single block device - partition tables, 
 * partition sizes, major number, etc.                                */
struct brd_device {
   int brd_number;

   struct request_queue	*brd_queue;
   struct gendisk	*brd_disk;   /* keep track of disk partitions */
   struct list_head	 brd_list;

   /* Backing store of pages and lock to protect it. */
   spinlock_t		   brd_lock;
   struct radix_tree_root  brd_pages;  
};

static DEFINE_MUTEX(brd_mutex);

/* Look up and return a brd's page for a given sector. */
static struct page *brd_lookup_page(struct brd_device *brd, sector_t sector) {
   pgoff_t idx;
   struct page *page;

   /* The page lifetime is protected by the fact that the device node has 
    * been opened -- so locking is not really needed.
    *
    * This is strictly true for the radix-tree nodes as well (ie. the
    * rcu_read_lock() is not actually needed).  However, that is not a
    * documented feature of the radix-tree API so it is better to be
    * safe here (total exclusion from radix tree updates is not guaranteed,
    * only from deletes).
    *
    * rcu_read_lock - used by a reader to inform the reclaimer that the 
    * reader is entering an RCU read-side critical section.  It is illegal 
    * to block while in an RCU read-side critical section.  Any 
    * RCU-protected data structure accessed during an RCU read-side 
    * critical section is guaranteed to remain unreclaimed for the full 
    * duration of that critical section.
    */
   rcu_read_lock();
   idx = sector >> PAGE_SECTORS_SHIFT;      /* xlate sector to page index */
   page = radix_tree_lookup(&brd->brd_pages, idx);
   rcu_read_unlock();
   
   BUG_ON(page && page->index != idx);

   return page;
}

/* Look up and return a brd's page for a given sector.
 * If one does not exist, allocate an empty page, and insert that. Then
 * return it.  */
static struct page *brd_insert_page(struct brd_device *brd, sector_t sector) {
   pgoff_t idx;
   struct page *page;
   gfp_t gfp_flags;         /* gfp = get free page */
   
   page = brd_lookup_page(brd, sector);
   if (page) return page;

   /* Must use NOIO because otherwise execution may recurse back into the
    * block or filesystem layers from page reclaim.
    *
    * Cannot support XIP and highmem, because the ->direct_access
    * routine for XIP must return memory that is always addressable.
    * If XIP was reworked to use pfns and kmap throughout, this
    * restriction might be able to be lifted.
    */
   gfp_flags = GFP_NOIO | __GFP_ZERO;
   gfp_flags |= __GFP_HIGHMEM;
   page = alloc_page(gfp_flags);
   if (!page) return NULL;
   
   if (radix_tree_preload(GFP_NOIO)) {  /* check if there is enough memory  */
      __free_page(page);                /* for the next page to be inserted */
      return NULL;                      /* into the radix tree              */
   }

   spin_lock(&brd->brd_lock);
   idx = sector >> PAGE_SECTORS_SHIFT;
   page->index = idx;
   if (radix_tree_insert(&brd->brd_pages, idx, page)) {  /* idx is the key */
      __free_page(page);
      page = radix_tree_lookup(&brd->brd_pages, idx);  /* lookup by index */
      BUG_ON(!page);
      BUG_ON(page->index != idx);
   }
   spin_unlock(&brd->brd_lock);
   
   radix_tree_preload_end();

   return page;
}

static void brd_zero_page(struct brd_device *brd, sector_t sector) {
   struct page *page;
   
   page = brd_lookup_page(brd, sector);
   if (page) clear_highpage(page);
}

/* Free all backing store pages and radix tree. This must only be called when
 * there are no other users of the device.  */
#define FREE_BATCH 16
static void brd_free_pages(struct brd_device *brd) {
   unsigned long pos = 0;
   struct page *pages[FREE_BATCH];
   int nr_pages;

   do {
      int i;

      /* collect up to FREE_BATCH pages starting at index pos, put the *
       * results in array pages                                        */
      nr_pages = radix_tree_gang_lookup(&brd->brd_pages,
					(void**)pages, pos, FREE_BATCH);

      for (i = 0; i < nr_pages; i++) {
	 void *ret;

	 BUG_ON(pages[i]->index < pos);
	 pos = pages[i]->index;
	 ret = radix_tree_delete(&brd->brd_pages, pos);
	 BUG_ON(!ret || ret != pages[i]);
	 __free_page(pages[i]);
      }
      
      pos++;
      
      /* This assumes radix_tree_gang_lookup always returns as
       * many pages as possible. If the radix-tree code changes,
       * so will this have to.
       */
   } while (nr_pages == FREE_BATCH);
}

/* copy_to_brd_setup must be called before copy_to_brd. It may sleep.  */
static int copy_to_brd_setup(struct brd_device *brd, sector_t sector, size_t n){
   unsigned int offset = (sector & (PAGE_SECTORS-1)) << SECTOR_SHIFT;
   size_t copy;

   copy = min_t(size_t, n, PAGE_SIZE - offset);
   if (!brd_insert_page(brd, sector)) return -ENOMEM;
   if (copy < n) {
      sector += copy >> SECTOR_SHIFT;
      if (!brd_insert_page(brd, sector)) return -ENOMEM;
   }
   return 0;
}

static void discard_from_brd(struct brd_device *brd,
			     sector_t sector, size_t n) {
   while (n >= PAGE_SIZE) {
      /* Don't want to actually discard pages here because
       * re-allocating the pages can result in writeback
       * deadlocks under heavy load - so brd_zero_page is called.  */
      brd_zero_page(brd, sector);
      sector += PAGE_SIZE >> SECTOR_SHIFT;
      n -= PAGE_SIZE;
   }
}

/* Copy n bytes from src to the brd starting at sector.  Atomic. 
 *
 * On 32 bit systems only 4GB is addressable but a system may have more 
 * RAM than that.  To allow the kernel to reach the higher address space
 * a mechanism called kmap() is employed to temporarily map high memory
 * to kernel space.  Below this is done to perform a simple copy.
 *
 * kmap_atomic is used because it is fast: it applies only to the CPU
 * where the mapping takes place to avoid syncing with other CPUs and
 * it uses a small subset of kernel address ranges.  Since the mapping 
 * is restricted to the CPU that issued it, the issuing task is 
 * required to stay on that CPU until it has finished, or else some 
 * other task may displace its mappings.  Therefore the operation
 * must be done atomically and protect some copy and that's about it.
 *
 * It may be assumed that k[un]map_atomic() won't fail.  */
static void copy_to_brd(struct brd_device *brd, const void *src,
			sector_t sector, size_t n) {
   struct page *page;
   void *dst;
   unsigned int offset = (sector & (PAGE_SECTORS-1)) << SECTOR_SHIFT;
   size_t copy;
   
   copy = min_t(size_t, n, PAGE_SIZE - offset);
   page = brd_lookup_page(brd, sector);
   BUG_ON(!page);

   dst = kmap_atomic(page);
   memcpy(dst + offset, src, copy);
   kunmap_atomic(dst);

   if (copy < n) {
      src += copy;
      sector += copy >> SECTOR_SHIFT;
      copy = n - copy;
      page = brd_lookup_page(brd, sector);
      BUG_ON(!page);
      
      dst = kmap_atomic(page);
      memcpy(dst, src, copy);
      kunmap_atomic(dst);
   }
}

/* Copy n bytes to dst from the brd starting at sector.  Atomic. 
 *
 * kmap_atomic is used again.  */
static void copy_from_brd(void *dst, struct brd_device *brd,
			  sector_t sector, size_t n) {
   struct page *page;
   void *src;
   unsigned int offset = (sector & (PAGE_SECTORS-1)) << SECTOR_SHIFT;
   size_t copy;

   copy = min_t(size_t, n, PAGE_SIZE - offset);
   page = brd_lookup_page(brd, sector);
   if (page) {
      src = kmap_atomic(page);
      memcpy(dst, src + offset, copy);
      kunmap_atomic(src);
   } else
      memset(dst, 0, copy);

   if (copy < n) {
      dst += copy;
      sector += copy >> SECTOR_SHIFT;
      copy = n - copy;
      page = brd_lookup_page(brd, sector);
      if (page) {
	 src = kmap_atomic(page);
	 memcpy(dst, src, copy);
	 kunmap_atomic(src);
      } else
	 memset(dst, 0, copy);
   }
}

/* Process a single bvec of a bio. 
 * 
 * Uses kmap_atomic  */
static int brd_do_bvec(struct brd_device *brd, struct page *page,
		       unsigned int len, unsigned int off, int rw,
		       sector_t sector) {
   void *mem;
   int err = 0;
   
   if (rw != READ) {
      err = copy_to_brd_setup(brd, sector, len);
      if (err) goto out;
   }

   mem = kmap_atomic(page);
   if (rw == READ) {
      copy_from_brd(mem + off, brd, sector, len);
      flush_dcache_page(page);
   } else {
      flush_dcache_page(page);
      copy_to_brd(brd, mem + off, sector, len);
   }
   kunmap_atomic(mem);

 out:
   return err;
}

/* The custom 'request' function 
 *
 * 'unlikely' is a macro that instructs the compiler to emit instructions 
 * that will cause branch prediction to favor the "likely" side of a jump 
 * instruction.  This can be a big win, if the prediction is correct it 
 * means that the jump instruction will take zero cycles.  */
static void brd_make_request(struct request_queue *q, struct bio *bio) {
   struct block_device *bdev = bio->bi_bdev;
   struct brd_device *brd = bdev->bd_disk->private_data;
   int rw;
   struct bio_vec *bvec;
   sector_t sector;
   int i;
   int err = -EIO;

   sector = bio->bi_sector;
   if (bio_end_sector(bio) > get_capacity(bdev->bd_disk)) goto out;

   if (unlikely(bio->bi_rw & REQ_DISCARD)) {
      err = 0;
      discard_from_brd(brd, sector, bio->bi_size);
      goto out;
   }

   rw = bio_rw(bio);
   if (rw == READA) rw = READ;
   
   /* macro - iterate through all bvecs, in this case do 'brd_do_bvec' */
   bio_for_each_segment(bvec, bio, i) {
      unsigned int len = bvec->bv_len;
      err = brd_do_bvec(brd, bvec->bv_page, len, bvec->bv_offset, rw, sector);
      if (err) break;
      sector += len >> SECTOR_SHIFT;
   }

 out:
   /* inform the block I/O subsystem that I/O op is complete */
   bio_endio(bio, err);
}

static int brd_direct_access(struct block_device *bdev, sector_t sector,
			     void **kaddr, unsigned long *pfn) {
   struct brd_device *brd = bdev->bd_disk->private_data;
   struct page *page;
   
   printk("brd_direct_access:\n");
   if (!brd) return -ENODEV;
   if (sector & (PAGE_SECTORS-1)) return -EINVAL;
   if (sector + PAGE_SECTORS > get_capacity(bdev->bd_disk)) return -ERANGE;
   page = brd_insert_page(brd, sector);
   if (!page) return -ENOMEM;
   *kaddr = page_address(page);
   *pfn = page_to_pfn(page);

   return 0;
}

static const struct block_device_operations brd_fops = {
   .owner =         THIS_MODULE,
   .direct_access = brd_direct_access,
};

/*
 * And now the modules code and kernel interface.
 */
static int rd_nr = 1;
int rd_size = 16384;   /* 16MB ram */

/* The device scheme is derived from loop.c. Keep them in synch where possible
 * (should share code eventually).   */
static LIST_HEAD(brd_devices);
static DEFINE_MUTEX(brd_devices_mutex);

static struct brd_device *brd_alloc(void) {
   struct brd_device *brd;
   struct gendisk *disk;

   brd = kzalloc(sizeof(*brd), GFP_KERNEL);  /* more efficient kcalloc */
   if (!brd) goto out;
   brd->brd_number = 0;
   spin_lock_init(&brd->brd_lock);
   INIT_RADIX_TREE(&brd->brd_pages, GFP_ATOMIC); /* ops performed atomically */

   /* Use 'blk_alloc_queue' to allocate a request queue that is used with 
    * a custom make_request function.  That function should be set with 
    * 'blk_queue_make_request'.  */
   brd->brd_queue = blk_alloc_queue(GFP_KERNEL);
   if (!brd->brd_queue) goto out_free_dev;
   blk_queue_make_request(brd->brd_queue, brd_make_request);  

   /* set max number of sectors for a request of this kind */
   blk_queue_max_hw_sectors(brd->brd_queue, 1024);
   /* set bounce buffer limit for this queue */
   blk_queue_bounce_limit(brd->brd_queue, BLK_BOUNCE_ANY); /* never bounce   */
   
   brd->brd_queue->limits.discard_granularity = PAGE_SIZE; /* discard pages  */
   brd->brd_queue->limits.max_discard_sectors = UINT_MAX;  /* max sects dscd */
   brd->brd_queue->limits.discard_zeroes_data = 1;
   queue_flag_set_unlocked(QUEUE_FLAG_DISCARD, brd->brd_queue); /* set flag  */
   
   /* allocate space for disk (of type struct gendisk*) */
   disk = brd->brd_disk = alloc_disk(1);  /* one minor # */
   if (!disk) goto out_free_queue;
   disk->major		= RAMDISK_MAJOR;
   disk->first_minor	= 0;
   disk->fops		= &brd_fops;
   disk->private_data	= brd;
   disk->queue		= brd->brd_queue;
   disk->flags |= GENHD_FL_SUPPRESS_PARTITION_INFO;
   sprintf(disk->disk_name, "ram0");
   set_capacity(disk, rd_size * 2);

   return brd;

 out_free_queue:
   blk_cleanup_queue(brd->brd_queue);
 out_free_dev:
   kfree(brd);
 out:
   return NULL;
}

static void brd_free(struct brd_device *brd) {
   put_disk(brd->brd_disk);    /* decrease reference count on disk */
   blk_cleanup_queue(brd->brd_queue);
   brd_free_pages(brd);
   kfree(brd);
}

static struct brd_device *brd_init_one(void) {
   struct brd_device *brd;
   
   list_for_each_entry(brd, &brd_devices, brd_list) {
      if (brd->brd_number == 0) goto out;
   }
   
   if ((brd = brd_alloc())) {
      add_disk(brd->brd_disk);   /* increase reference count on disk */
      list_add_tail(&brd->brd_list, &brd_devices);
   }
 out:
   return brd;
}

static void brd_del_one(struct brd_device *brd) {
   list_del(&brd->brd_list);
   del_gendisk(brd->brd_disk);
   brd_free(brd);
}

static struct kobject *brd_probe(dev_t dev, int *part, void *data) {
   struct brd_device *brd;
   struct kobject *kobj;

   mutex_lock(&brd_devices_mutex);
   brd = brd_init_one();
   kobj = brd ? get_disk(brd->brd_disk) : ERR_PTR(-ENOMEM);
   mutex_unlock(&brd_devices_mutex);
   
   *part = 0;
   return kobj;
}

int init_module (void) {
   unsigned long range = rd_nr;
   struct brd_device *brd, *next;
   
   if (register_blkdev(RAMDISK_MAJOR, "ramdisk")) return -EIO;
   
   if (!(brd = brd_alloc())) goto out_free;
   list_add_tail(&brd->brd_list, &brd_devices);
   list_for_each_entry(brd, &brd_devices, brd_list) add_disk(brd->brd_disk);
   
   blk_register_region(MKDEV(RAMDISK_MAJOR, 0), range,
		       THIS_MODULE, brd_probe, NULL, NULL);
   
   printk(KERN_INFO "brd: module loaded\n");
   return 0;
   
 out_free:
   list_for_each_entry_safe(brd, next, &brd_devices, brd_list) {
      list_del(&brd->brd_list);
      brd_free(brd);
   }
   unregister_blkdev(RAMDISK_MAJOR, "ramdisk");
   
   return -ENOMEM;
}

void cleanup_module(void) {
   unsigned long range;
   struct brd_device *brd, *next;
   
   range = rd_nr ? rd_nr : 1UL << MINORBITS;
   
   list_for_each_entry_safe(brd, next, &brd_devices, brd_list)
      brd_del_one(brd);
   
   blk_unregister_region(MKDEV(RAMDISK_MAJOR, 0), range);
   unregister_blkdev(RAMDISK_MAJOR, "ramdisk");
}

MODULE_LICENSE("GPL");

