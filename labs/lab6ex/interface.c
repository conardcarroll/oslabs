#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>     /* unregister_chrdev_region, includes wait.h */
#include <linux/cdev.h>   /* cdev_add, cdev_del */
#include <asm/uaccess.h>  /* copy_from_user, copy_to_user */
#include <linux/sched.h>  /* TASK_INTERRUPTIBLE, includes rwsem.h, kernel.h */

/*
   When a shared resource is only read from and not written to, there
   is no change to the data so if multiple processes want to only read
   from the shared resource there should not be any problem in
   allowing access to all readers.  Linux provides a special
   reader/writer semaphore that allows multiple readers into the
   critical section but only one writer at any given time.

   Every time the read function is entered a read/write semaphore is 
   held and a message is printed to indicate that the semaphore was held 
   successfully - then the read process reads and does a wait.  In the 
   write function the same read/write semaphore is held but if there 
   are processes reading the device the write process sleeps until the 
   reads complete.
*/

wait_queue_head_t queue;
int Major;
int bytes_written, bytes_read;

struct device {
   char array[100];
   struct rw_semaphore rwsem;
} char_arr;

int open(struct inode *inode, struct file *filp) {
   printk(KERN_INFO "open: read=%d written=%d\n", bytes_read, bytes_written); 
   bytes_read = bytes_written;
   return 0;
}

int release(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "release: read=%d written=%d\n",bytes_read, bytes_written);
    return 0;
}

ssize_t read(struct file *filp, char *buff, size_t cnt, loff_t *offp) {
   unsigned long ret;
   printk("read: read=%d written=%d count=%d\n", bytes_read,bytes_written,cnt);
   printk("read: buffer at start=%s\n",buff);

   down_read(&char_arr.rwsem);
   printk("read: through semaphore\n");
   wait_event_timeout(queue, 0, 15*HZ);
   printk("read: through the timer\n");
   /** comment the next two lines so 2 readers will see the same # bytes **/
   /** if (cnt > bytes_read) cnt = bytes_read; **/
   /** bytes_read = bytes_read - cnt; **/
   ret = copy_to_user(buff, char_arr.array, cnt);
   printk("read: done reading\n");
   up_read(&char_arr.rwsem);
   return cnt;
}

ssize_t write(struct file *filp, const char *buff, size_t cnt, loff_t *offp){
   unsigned long ret; 
   printk(KERN_INFO "write: entering\n");
   down_write(&char_arr.rwsem);
   wait_event_timeout(queue, 0, 10*HZ);
   cnt = (cnt > 99) ? 99 : cnt;
   ret = copy_from_user(char_arr.array, buff, cnt); 
   bytes_written += cnt;
   printk("write: read=%d written=%d cnt=%d\n", bytes_read, bytes_written, cnt);
   printk("write: %s\n",buff);
   up_write(&char_arr.rwsem);
   return cnt;
}

struct file_operations fops = {
   .read = read,
   .write = write,
   .open = open,
   .release = release
};

struct cdev *arr_cdev; 

int init_module (void) {
   int ret;
   dev_t dev_no;

   printk("---------------------------------------\n");

   arr_cdev = cdev_alloc();    
   arr_cdev->ops = &fops;
   arr_cdev->owner = THIS_MODULE;
   printk(KERN_INFO "init:\n");

   /*
   Major = 250;
   ret = register_chrdev(Major, "interface", &fops);
   if (ret < 0) {
      printk(KERN_INFO "Major number allocation is failed\n");
      return ret;    
   }
   */

   ret = alloc_chrdev_region( &dev_no , 0, 1, "interface");
   if (ret < 0) {
      printk(KERN_INFO "Major number allocation is failed\n");
      return ret;    
   }
   Major = MAJOR(dev_no);
   if (MKDEV(Major,0) != dev_no)
      printk(KERN_INFO "Yikes - something is wrong!!\n");
   
   printk (KERN_INFO "init: Major %d\n", Major);
   ret = cdev_add(arr_cdev, dev_no, 1);
   if (ret < 0) {
      printk(KERN_INFO "Unable to allocate cdev");
      return ret;
   }
   init_rwsem(&char_arr.rwsem);
   init_waitqueue_head(&queue);
    
   return 0;
}

void cleanup_module(void) {
   printk(KERN_INFO "cleanup:\n");
   cdev_del(arr_cdev);
   unregister_chrdev_region(Major, 1);
   unregister_chrdev(Major, "interface");
}

MODULE_LICENSE("GPL");    