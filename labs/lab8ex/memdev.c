/*
 *  chardev.c - Create an input/output character device
 *  http://linux.die.net/lkmpg/x892.html
 *
 *  Note: if an attempt is made to use get_user or put_user to transfer a
 *  value that does not fit one of the specific sizes, the result is usually
 *  a strange message from the compiler, such as "conversion to non-scalar
 *  type requested." In such cases, 'copy_to_user' or 'copy_from_user' must
 *  be used.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for get_user and put_user */

#include "memdev.h"
static int Device_Open = 0;
/* Called whenever a process attempts to open the device file */
static int open(struct inode *inode, struct file *file) {
   printk(KERN_INFO "open(%p)\n", file);

   /* Do not talk to two processes at the same time */
   if (Device_Open) return -EBUSY; else Device_Open++;
   return 0;
}

static int release(struct inode *inode, struct file *file) {
   printk(KERN_INFO "release(%p,%p)\n", inode, file);

   /* Clear Device_open - ready for the next caller */
   Device_Open--;
   return 0;
}

/* Called when a user process attempts to read from the the device   */
static ssize_t read(struct file *file,   /* see include/linux/fs.h   */
		    char *buffer,        /* buffer to be             *
				          * filled with data         */
		    size_t length,       /* length of the buffer     */
		    loff_t *offset) {
   /* Number of bytes actually written to the buffer */
   int bytes_read = 0;
   printk(KERN_INFO "read(%p,%p,%d)\n", file, buffer, length);

   /* Return 0 if at end of file  */
   if (*Message_Ptr == 0) return 0;

   /* Put the data into the buffer */
   while (length && *Message_Ptr) {

      /* Because the buffer is in the user data segment, not the kernel data
       * segment, assignment does not work.  Instead, put_user is used to
       * copy data from the kernel data segment to the user data segment. */
      put_user(*(Message_Ptr++), buffer++);
      length--;
      bytes_read++;
   }

   printk(KERN_INFO "Read %d bytes, %d left\n", bytes_read, length);

   /* Read functions must return the number of bytes inserted into the buffer */
   return bytes_read;
}

/* Called when a user process attempts to write to the device file. */
static ssize_t write(struct file *file,
		     const char *buffer,
		     size_t length,
		     loff_t *offset) {
   int i;
   void* val;

   printk(KERN_INFO "write(%p,%s,%d)", file, buffer, length);

   for (i = 0; i < length && i < BUF_LEN; i++) get_user(Message[i], buffer+i);
   Message_Ptr = Message;

   size_t l = min(length, (size_t)MEM_SIZE);
   val = copy_from_user(ptr, buffer, length);

   /* The number of input characters used is returned  */
   return l;
}

/* Called when a process tries to do an ioctl on the device file.
   There are two parameters in addition to the file struct: the number
   of the ioctl called and the parameter given to the ioctl function.

   If the ioctl is write or read/write, output from the function is
   returned to the calling process  */
long ioctl(struct file *file,
	   unsigned int ioctl_num,	/* number and param for ioctl */
	   unsigned long ioctl_param) {
   int i;
   char *temp;
   char ch;

   /* Switch according to the ioctl called */
   switch (ioctl_num) {
   case IOCTL_GET_MEM:
   break;

   case IOCTL_FREE_MEM:
   break;

   case IOCTL_WRITE_MEM:
      break;
   case IOCTL_WRITE_MEM:
   break;
   }

   return 0;
}

/* Module Declarations */

/* This structure holds the functions to be called when a process acts
   on the device.  Since a pointer to this structure is kept in the devices
   table, it can't be local to init_module.  NULL is for unimplemented
   functions. */
struct file_operations Fops = {
   .read = read,
   .write = write,
   .unlocked_ioctl = ioctl,
   .open = open,
   .release = release
};

/* Initialize the module - Register the character device */
int init_module(void) {
   int ret_val;
   /* Register the character device */
   ret_val = register_chrdev(MAJOR_NUM, DEVICE_NAME, &Fops);
   if (ret_val < 0) {
      printk(KERN_ALERT "%s failed with %d\n",
	     "Sorry, registering the device ", ret_val);
      return ret_val;
   }
	
	ret_val = buddy_init(MEM_SIZE);

   printk(KERN_INFO "%s The major device number is %d.\n",
	  "Registration is a success", MAJOR_NUM);
   return ret_val;
}

/* Cleanup - unregister the appropriate file from /proc  */
void cleanup_module(void) {
	buddy_kill();
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
	printk(KERN_INFO, "Unregistered memdev\n");
}