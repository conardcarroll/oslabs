/* add something to allow 'cat' to print only what is interesting    */
/* change the return value of read function to reflect the data size */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

static int Major;
static int written, read;   /****  The new stuff ****/
                            /* written - how big is the area containing data
                               read - counts down on read to 0 to stop print */
/* Sample device driver

   The device is just an array that can hold a maximum of 100 characters 
   and its concurrent access can be controlled using the semaphore "sem".

   The following 4 operations for the device will be implemented:
      Open, Read, Write, Close.

   The 'cdev' struct is used to keep track of all the character devices:
      struct cdev *kernel_cdev; 

   This is the name of the character device

   Char devices are accessed through names in the filesystem. Those
   names are called special files or device files or simply nodes of
   the filesystem tree; they are conventionally located in the /dev
   directory. Special files for char drivers are identified by a "c"
   in the first column of the output of ls -l.  Block devices appear in
   /dev as well, but they are identified by a "b." 

   Ex: crw-rw-rw-    1 root     root       1,   3 Apr 11  2002 null
   has Major number 1, minor number 3

   The major number identifies the driver associated with the device.
   The minor number is used by the kernel to determine exactly which 
   device is being referred to

   Major numbers can be allocated dynamically using this function:
      int alloc_chrdev_region(dev_t *dev, 
                              unsigned int fmin,
                              unsigned int count, 
                              char *name); 

   The arguments are:
      dev ->   The dev_t variable type, gets the major number that the kernel 
               allocates
      fmin ->  The first minor number in case a series of minor numbers is
               desired
      count -> The number of contiguous sets of major numbers that are
               to be allocated. 
      name ->  Name of the device - will appear in /proc/devices

   Minor numbers will be allocated starting from 0 (common practice). 
   Only one device number needs to be allocated since this is a single 
   driver for one device so count=1
   Name = 'char_arr_dev' 

*/
struct device {
   char array[100];
   struct semaphore sem;
} char_arr;

/*
   Linux uses a cdev structure to keep track of all the character devices, 
   struct cdev {
      struct kobject kobj;
      struct module *owner;
      const struct file_operations *ops;
      struct list_head list;
      dev_t dev;
      unsigned int count;
   };   
*/
struct cdev *arr_cdev; 

/* 
   The open method is provided for a driver to do any initialization in 
   preparation for later operations. In most drivers, open should perform 
   the following tasks:
      -> Check for device-specific errors (such as device-not-ready or 
         similar hardware problems)
      -> Initialize the device if it is being opened for the first time
      -> Update the f_op pointer, if necessary
      -> Allocate and fill any data structure to be put in filp->private_data
   The first order of business, however, is usually to identify which 
   device is being opened

   An inode stores all information about a file system object 
   (file, directory, device node, socket, pipe, etc)

   The semaphore holds this device open to completion - another call to this
   device would fail (-1 is returned)
*/
int open(struct inode *inode, struct file *filp) {
   if (down_interruptible(&char_arr.sem)) { 
      printk(KERN_INFO "could not hold semaphore"); 
      return -1;
   } 
   printk(KERN_INFO "open: read=%d written=%d\n",read,written); 
   read = written;       /**** Start with read equal to written!! ****/
   return 0;
}

/*
   The role of the release method is the reverse of open.  The device 
   method should perform the following tasks:
      -> Deallocate anything that open allocated in filp->private_data
      -> Shut down the device on last close
*/
int release(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "release: read=%d written=%d\n",read,written);
    up(&char_arr.sem);
    return 0;
}

/* reading continues until the return value of read_char is 0 
   recall read = written at the start
*/
ssize_t read_char (struct file *filp, char *b, size_t cnt, loff_t *o) {
   unsigned long ret;
   printk("read_char: read=%d written=%d count=%d\n", read, written, cnt);
   if (cnt > read) cnt = read;
   read = read - cnt;
   ret = copy_to_user(b, char_arr.array, cnt);
   return cnt;
}

ssize_t write_char(struct file *filp, const char *b, size_t cnt, loff_t *o) { 
   unsigned long ret; 
   cnt = (cnt > 99) ? 99 : cnt;
   ret = copy_from_user(char_arr.array, b, cnt); 
   written += cnt;
   printk(KERN_INFO "write_char: read=%d written=%d cnt=%d\n",read,written,cnt);
   return cnt;
}

struct file_operations fops = { 
   .owner = THIS_MODULE,
   .read = read_char, 
   .write = write_char, 
   .open = open, 
   .release = release
};

int init_module (void) {
   int ret;
   dev_t dev_no;

   printk("---------------------------------------\n");

   /* allocate a cdev structure */
   arr_cdev = cdev_alloc();    
   arr_cdev->ops = &fops;            // allowed file operations
   arr_cdev->owner = THIS_MODULE;    // this module
   printk(KERN_INFO "init: starting driver\n");

   /* allocate a major number dynamically */
   ret = alloc_chrdev_region(&dev_no, 0, 1, "char_arr_dev");
   if (ret < 0) {
      printk("Major number allocation is failed\n");
      return ret;    
   }
   /* dev_no holds the combination major number that the kernel has 
      allocated and the first user selected minor number
      The following extracts that information from dev_no:
   */
   Major = MAJOR(dev_no);
   /* dev will be used to register the driver */
   if (MKDEV(Major,0) != dev_no)
      printk(KERN_INFO "Yikes - something is wrong!!\n");

   printk(KERN_INFO "init: Major %d\n", Major);
   /* register the driver - should show up in /proc/devices */
   ret = cdev_add(arr_cdev, dev_no, 1);
   if (ret < 0 ) {
      printk(KERN_INFO "Unable to allocate cdev");
      return ret;
   }

   sema_init(&char_arr.sem, 1);

   return 0;
}

void cleanup_module(void) {
   printk(KERN_INFO "cleanup: unloading driver\n");
   cdev_del(arr_cdev);
   unregister_chrdev_region(Major, 1);
   unregister_chrdev(Major, "arr_dev_04");
}
