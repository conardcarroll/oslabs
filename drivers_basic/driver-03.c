#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>      /* copy_to_user  copy_from_user */

int fudge = 0;
static int Major;

/* Sample device driver

   The device is just an array that can hold a maximum of 100 characters 
   and its concurrent access can be controlled using the semaphore "sem".

   The following 4 operations for the device will be implemented:
      Open, Read, Write, Close.

   The 'cdev' struct is used to keep track of all the character devices:
      struct cdev *kernel_cdev; 

   struct cdev {
      struct kobject kobj;
      struct module *owner;
      const struct file_operations *ops;
      struct list_head list;
      dev_t dev;
      unsigned int count;
   };   

   This is the name of the character device: char_arr_dev

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
               allocates (just an int)
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

struct cdev *arr_cdev; 

/* 
   The open method is provided for a driver to do any initialization in 
   preparation for later operations. In most drivers, open should perform 
   the following tasks:
      -> Check for device-specific errors (such as device-not-ready or 
         similar hardware problems)
      -> Initialize the device if it is being opened for the first time
      -> Update the fop (file ops) pointer, if necessary
      -> Allocate and fill any data structure to be put in filp->private_data
      -> Identify which device is being opened

   An inode stores all information about a file system object 
   (file, directory, device node, socket, pipe, etc)

   The semaphore holds this device open to completion - another call to this
   device would fail (-1 is returned)
*/
int open(struct inode *inode, struct file *filp) {
   if (down_interruptible(&char_arr.sem)) { 
      printk(KERN_INFO "open: could not hold the semaphore"); 
      return -1;
   } 
   printk(KERN_INFO "open: opening device"); 
   return 0;
}

/*
   The role of the release method is the reverse of open.  The device 
   method should perform the following tasks:
      -> Deallocate anything that open allocated in filp->private_data
      -> Shut down the device on last close
*/
int release(struct inode *inode, struct file *filp) {
    printk (KERN_INFO "release: closing device\n");
    up(&char_arr.sem);
    return 0;
}

/*
   b is the userspace buffer to put data into, 
   c is the number of bytes to read
   copy_to_user is the interface between kernel space and user space
   which behaves like normal memcpy

   The user pages being addressed might not be currently present in memory, 
   and the virtual memory subsystem can put the process to sleep while the 
   page is being transferred into place.  This happens, for example, when 
   the page must be retrieved from swap space. The net result for the driver 
   writer is that any function that accesses user space must be reentrant, 
   must be able to execute concurrently with other driver functions, and, in
   particular, must be in a position where it can legally sleep.

   Hence a semaphore is used to protect this operation

   The return value for read is interpreted by the calling application program:
      -> If the value equals the count argument passed to the read system 
         call, the requested number of bytes has been transferred. 
      -> If the value is positive, but smaller than count, only part of the 
         data has been transferred. This may happen for a number of reasons, 
         depending on the device.  Most often, the application program retries 
         the read.  For instance, if you read using the fread function, the 
         library function reissues the system call until completion of the 
         requested data transfer.
      -> If the value is 0, end-of-file was reached (and no data was read)
      -> A negative value means there was an error. The value specifies what 
         the error was, according to <linux/errno.h>. Typical values returned 
         on error include -EINTR (interrupted system call) or -EFAULT (bad 
         address).
   If there is no data, but it may arrive later, the read system call should 
   block. 
*/
ssize_t read(struct file *f, char *b, size_t cnt, loff_t *o) {
   unsigned long ret;

   if (fudge) {
      printk(KERN_INFO "read: end of transmission\n");
      fudge = 0;
      return 0;
   }
   printk (KERN_INFO "read: user data to device, cnt=%d sz=%d\n",cnt,strlen(b));
   ret = copy_to_user(b, char_arr.array, cnt);
   fudge = 1;
   return cnt;
}

/* 
   write, like read, can transfer less data than was requested, according 
   to the following rules for the return value:
      -> If the value equals count, the requested number of bytes has been 
         transferred.
      -> If the value is positive, but smaller than count, only part of the 
         data has been transferred. The program will most likely retry writing 
         the rest of the data.
      -> If the value is 0, nothing was written. This result is not an error, 
         and there is no reason to return an error code. Once again, the 
         standard library retries the call to write.
      -> A negative value means an error occurred; as for read, valid error 
         values are those defined in <linux/errno.h>.
*/
ssize_t write(struct file *f, const char *b, size_t cnt, loff_t *o) {
   unsigned long ret; 
   printk(KERN_INFO "write: device data to user, cnt=%d\n", cnt);
   ret = copy_from_user(char_arr.array, b, cnt); 
   return cnt;
}

/* this struct is huge - see include/linux/fs.h 
   purpose is to connect operations to the device numbers 
   format for entry of data is kernel specific
 
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
   int ret, i;
   dev_t dev_no;

   printk("---------------------------------------\n");

   for (i=0 ; i < 100 ; i++) char_arr.array[i] = 0;

   /* allocate a cdev structure */
   arr_cdev = cdev_alloc();    
   arr_cdev->ops = &fops;            // allowed file operations
   arr_cdev->owner = THIS_MODULE;    // this module
   printk(KERN_INFO "init: starting driver\n");

   /* allocate a major number dynamically - dev_no gets it,
      first minor is 0, only 1 major, name is char_arr_dev   */
   ret = alloc_chrdev_region(&dev_no, 0, 1, "arr_dev_03");
   if (ret < 0) {
      printk(KERN_INFO "Major number allocation is failed\n");
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
   /* register the driver - should show up in /proc/devices 
      1 is the number of consecutive minors */
   if ((ret=cdev_add(arr_cdev, dev_no, 1)) < 0) {
      printk(KERN_INFO "Unable to allocate cdev");
      return ret;
   }

   /* initialize the semaphore */
   sema_init(&char_arr.sem, 1);

   return 0;
}

void cleanup_module(void) {
   printk(KERN_INFO "cleanup: unloading driver\n");
   cdev_del(arr_cdev);
   unregister_chrdev_region(Major, 1);
   unregister_chrdev(Major, "arr_dev_03");
}

MODULE_LICENSE("GPL");
