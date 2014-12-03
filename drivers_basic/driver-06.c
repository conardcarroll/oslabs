#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <linux/cdev.h> 
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/wait.h>
#include <linux/time.h>

unsigned long copy_from_user (void*, const void*, unsigned long);
unsigned long copy_to_user (void*, const void*, unsigned long);

/* 
   The parallel port allows the input and output of digital information at
   the byte level.  The interface to the outside world is a female D-25 
   connector with twenty-five pins.  It is accessed internally at base
   address 0x378.  

   The programming interface used to access the I/O registry is made
   up of three functions:

      int check_region(unsigned long start, unsigned long len);
      struct resource *request_region(unsigned long, unsigned long, char *name);
      void release_region(unsigned long start, unsigned long len);

   check_region: may be called to see if a range of ports is available for 
      allocation; it returns a negative error code (such as -EBUSY or -EINVAL) 
      if the answer is no. 

   request_region: will actually allocate the port range, returning a 
      non-NULL pointer value if the allocation succeeds.  Drivers don't 
      need to use or save the actual pointer returned -- checking against 
      NULL is all you need to do.  The 'name' is the name of the region
      the module requests the region to be called by.

   release_region: releases the ports
*/

/* Major number and port number */
int Major = 61, port;

int open(struct inode *inode, struct file *filp) {  return 0;  }

int release(struct inode *inode, struct file *filp) {  return 0; }

ssize_t read(struct file *filp, char *buf, size_t c, loff_t *f_pos) {
   unsigned long ret;
 
   /* Buffer to read the device */
   char buffer;

   /* Read port */
   buffer = inb(0x378);
   printk(KERN_INFO "read: got %c\n",buffer);

   /* Transfer data to user space - ret is the number of bytes remaining
      to be transferred - hopefully this is 0 */
   ret = copy_to_user(buf, &buffer, 1); 
   printk(KERN_INFO "read: received %c\n",buffer);

   /* Whatever the amount of data the methods transfer, they should 
      generally update the file position at *offp to represent the 
      current file position after successful completion of the system call. 
      The kernel then propagates the file position change back into the 
      file structure when appropriate. */
   if (*f_pos == 0) { 
      *f_pos += 1; 
      return 1; 
   } else { 
      return ret; 
   }
}

ssize_t write(struct file *filp, const char *buf, size_t c, loff_t *f_p) {
   int ret;
   const char *tmp;

   /* Buffer write to device */
   char buffer;

   tmp = buf + c - 1;
   ret = copy_from_user(&buffer, tmp, 1);
   printk(KERN_INFO "write: got %c from user\n",buffer);

   /* Write to the port */
   outb(buffer, 0x378);
   printk(KERN_INFO "write: sent %c to printer port\n", buffer);

   return 1; 
}

/* Structure that declares the common */
/* file access fcuntions */
struct file_operations fops = { 
   .read = read,
   .write = write,
   .open = open,
   .release = release
};

int init_module(void) { 
   int result;

   /* Register device */
   result = register_chrdev(Major, "parlelport", &fops);
   if (result < 0) { 
      printk(KERN_INFO "parlelport: cannot obtain major number %d\n", Major); 
      return result; 
   } 
   
   /* Register port */
   port = check_region(0x378, 1);
   if (port) { 
      printk(KERN_INFO "parlelport: cannot reserve 0x378\n"); 
      result = port; 
      cleanup_module();
      return result;
   } 
   request_region(0x378, 1, "parlelport");

   printk(KERN_INFO "init: inserting parlelport module\n"); 
   printk(KERN_INFO "init: Major: %d\n", Major); 
   return 0;
}

void cleanup_module(void) {
   /* Make major number free */
   unregister_chrdev(Major, "parlelport");

   /* Make port free */ 
   if (!port) release_region(0x378, 1);

   printk(KERN_INFO "exit: removing parlelport module\n");
}

MODULE_LICENSE("GPL");
