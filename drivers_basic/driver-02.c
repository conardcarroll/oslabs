#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/fs.h>         /* register/unregister chardev */
#include <linux/cdev.h>       /* cdev_alloc, cdev_add */

static int Major;

/* Sample device driver

   The device is just an array that can hold a maximum of 100 characters 
   and its concurrent access can be controlled using the semaphore "sem".

   The following 4 operations for the device are implemented:
      Open, Read, Write, Close.

   The 'cdev' struct is used to keep track of all the character devices:
      struct cdev *arr_cdev; 

   arr_dev_02 is the name of the character device

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
   Name = 'arr_dev_02' 

*/
struct device {
   char array[100];
   struct semaphore sem;
} char_arr;

/*
   Linux uses a cdev structure to keep track of all the character devices, 
   struct cdev {
      struct kobject kobj;      *** shows up in the directory tree of /sys
      struct module *owner;
      const struct file_operations *ops;
      struct list_head list;
      dev_t dev;
      unsigned int count;
   };   
*/
struct cdev *arr_cdev; 

/* Stubs */
int open(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "Inside open \n");
    return 0;
}

int release(struct inode *inode, struct file *filp) {
    printk (KERN_INFO "Inside close \n");
    return 0;
}

/* Important! return value should be the bytes read, 0 when finished */
ssize_t read(struct file *f, char *b, size_t cnt, loff_t *o) {
   printk (KERN_INFO "Inside read \n");
   return 0;
}

/* Important!  Return value should be the bytes written */
ssize_t write(struct file *f, const char *b, size_t cnt, loff_t *o) {
   char buffer[128];
   cnt = (cnt > 64) ? 64 : cnt;
   strncpy(buffer,b,cnt);
   buffer[cnt-1] = 0;
   printk(KERN_INFO "Inside write, buffer=%s, size=%d\n", buffer, cnt);
   return cnt;
}

/* struct file_operations is huge - see include/linux/fs.h 
   purpose is to connect operations to the device numbers 
 
   fields: 
     struct module *owner - prevents module from being unloaded
            when its operations are in use
     open - open the device
     release - close the device
     read   - retrieves data from the device
     write  - sends data to the device
     llseek - change the current read/write cursor position in the file
     aio_read - asynchronous read - may not complete before the function returns
     readdir - for filesystems - to read directories
     poll - query whether read/write to some file descriptor will block
     ioctl - allow issue of device specific commands
     mmap - request a mapping of device memory to process address space
     flush - 
     fsync - another form of flush - slightly different from above
     lock  - implement file locking
     readv - read operation over many memory areas
     writev - write operation over many memory areas
     sendfile -
     sendpage -
     get_unmapped_area -
     check_flags -
     dir_notify -
*/
struct file_operations fops = { 
   .owner = THIS_MODULE,
   .read = read, 
   .write = write, 
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
   printk(KERN_INFO "Inside init module\n");

   /* allocate a major number dynamically */
   ret = alloc_chrdev_region(&dev_no, 0, 1, "arr_dev_02");
   if (ret < 0) {
      printk("Major number allocation is failed\n");
      return ret;    
   }
   /* dev_no holds the combination major number that the kernel has 
      allocated and the first user selected minor number
      The following extracts that information from dev_no:
   */
   Major = MAJOR(dev_no);
   /* dev will be used to register the driver (make it available) */
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
   printk(KERN_INFO "Inside cleanup_module\n");
   cdev_del(arr_cdev);
   unregister_chrdev_region(Major, 1);      /* return range of device numbers */
   unregister_chrdev(Major, "arr_dev_02");  /* release the major number */
}

MODULE_LICENSE("GPL");
