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
#include "buddy.h"

int current_idx = 0;    /* currently selected page */
int read_size = 0;      /* how many bytes to read when read ioctl is called */

/* Called whenever a process attempts to open the device file */
static int open(struct inode *inode, struct file *file) {
   printk(KERN_INFO "open(%p)\n", file);
   return 0;
}

static int release(struct inode *inode, struct file *file) {
   printk(KERN_INFO "release(%p,%p)\n", inode, file);
   return 0;
}

/* Called when a user process attempts to read from the the device   */
static ssize_t dev_read(struct file *file,   /* see include/linux/fs.h   */
		    char *buffer,        /* buffer to be             *
				          * filled with data         */
		    size_t length,       /* length of the buffer     */
		    loff_t *offset) {
				 int bytes = 0;
				 read_size = length;
             /* return error if trying to read more than the page size */
             if (length > buddy_size(current_idx)) {
                 printk(KERN_INFO "mem: read bigger than allocated area\n");
                 return -1;
             }

             /* reset the number of bytes read */
             bytes = 0;

             /* keep reading bytes until we've satisfied the request */
             while (bytes < read_size) {

                 /* get byte out of pool and send it to user */
                 put_user(*(buddy_pool+current_idx+bytes), (char*)buffer+bytes);

                 /* go to the next byte */
                 bytes++;
             }

             printk(KERN_INFO "mem: read %d bytes\n", bytes);

             /* return how many bytes were read */
             return bytes;

}

/* Called when a user process attempts to write to the device file. */
static ssize_t dev_write(struct file *file,
		     const char *buffer, 
		     size_t length, 
		     loff_t *offset) {
   int bytes;
	int size;
	char ch;
	

   /* reset the number of bytes written */
   bytes = 0;

   /* get the page size */
   size = buddy_size(current_idx);
	
	printk(KERN_INFO "mem: Entering memdev write\n");

   /* keep writing until some condition breaks us out */
   while (1) {

       /* get the next byte from userspace */
       get_user(ch, (char*)buffer+bytes);

       /* if it's a null terminator we are finished writing */
       if (ch == '\0') break;

       /* if we are about to write outside the page error out */
       if (bytes > size) {
           printk(KERN_INFO "mem: writing out of allocated area\n");
           return -1;
       }

       /* write the byte into the pool */
       *(buddy_pool+current_idx+bytes) = ch;
       printk(KERN_INFO "wrote %c to %d\n", ch, current_idx+bytes);

       /* go to the next byte */
       bytes++;
   }

   printk(KERN_INFO "mem: wrote %d bytes\n", bytes);

   /* return how many bytes were written */
   return bytes;

}

/* Called when a process tries to do an ioctl on the device file. 
   There are two parameters in addition to the file struct: the number 
   of the ioctl called and the parameter given to the ioctl function.

   If the ioctl is write or read/write, output from the function is 
   returned to the calling process  */
long ioctl(struct file *file,
	   unsigned int ioctl_num,	/* number and param for ioctl */
	   unsigned long ioctl_param) {
			int bytes;
			// char ch;
			int size;
			int write_len, read_len;
   
   /* Switch according to the ioctl called */
   switch (ioctl_num) {
   case IOCTL_GET_MEM:
   	printk(KERN_INFO "mem: allocating %d bytes\n", (int)ioctl_param);
   	return buddy_alloc((int)ioctl_param);
		
   break;
      
   case IOCTL_FREE_MEM:
		printk(KERN_INFO "mem: freeing idx %d\n", (int)ioctl_param);
		return buddy_free((int)ioctl_param);
   break;
   		//
		   case IOCTL_WRITE_MEM:
		// printk(KERN_INFO "mem: wrote %d bytes \n", bytes);
		// return write((int)ioctl_param);
         /* reset the number of bytes written */
         bytes = 0;

         /* get the page size */
         size = buddy_size(current_idx);
			printk("mem: about to write %s\n", (char*)ioctl_param);
			write_len = strlen((char*)ioctl_param);

         printk(KERN_INFO "mem: found string %d long\n", write_len);
         if (write_len > size) {
             printk(KERN_INFO "vmm: writing out of allocated area\n");
             return -1;
         }
			
			bytes = copy_from_user(buddy_pool+current_idx,(char*)ioctl_param, write_len + 1);

         // /* keep writing until some condition breaks us out */
 //         while (1) {
 //
 //             /* get the next byte from userspace */
 //             get_user(ch, (char*)ioctl_param+bytes);
 //
 //             /* if it's a null terminator we are finished writing */
 //             if (ch == '\0') break;
 //
 //             /* if we are about to write outside the page error out */
 //             if (bytes > size) {
 //                 printk(KERN_INFO "vmm: writing out of allocated area\n");
 //                 return -1;
 //             }
 //
 //             /* write the byte into the pool */
 //             *(buddy_pool+current_idx+bytes) = ch;
 //             printk(KERN_INFO "wrote %c to %d\n", ch, current_idx+bytes);
 //
 //             /* go to the next byte */
 //             bytes++;
 //         }

         printk(KERN_INFO "mem: could not write %d bytes\n", bytes);

         /* return how many bytes were written */
         return bytes;
		   break;
		   case IOCTL_READ_MEM:
         /* return error if trying to read more than the page size */
         if (read_size > buddy_size(current_idx)) {
             printk(KERN_INFO "mem: read bigger than allocated area\n");
             return -1;
         }
			
			read_len = strlen((char*)buddy_pool+current_idx);
			
			printk(KERN_INFO "mem: string %d in mem %s\n", read_len, (char*)buddy_pool+current_idx);

         /* reset the number of bytes read */
         bytes = 0;
			
			bytes = copy_to_user((char*)ioctl_param, buddy_pool+current_idx, read_len + 1);
			
			printk(KERN_INFO "mem: could not read %d bytes\n", bytes);
			
		   break;
   case IOCTL_SET_IDX:
	current_idx = (int)ioctl_param;
		printk(KERN_INFO "mem: set idx %d\n", (int)ioctl_param);
		return 0;
   break;

		case IOCTL_READ_SIZE:
		read_size = (int)ioctl_param;
		printk(KERN_INFO "mem: set read size %d\n", (int)ioctl_param);
		return 0;
		break;
	default:
	printk(KERN_INFO "unknown ioctl %d\n", ioctl_num);
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
   .read = dev_read,
   .write = dev_write,
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
	     "Sorry, registering the mem device ", ret_val);
      return ret_val;
   }
   
   printk(KERN_INFO "%s The major device number is %d.\n",
	  "Registration is a success", MAJOR_NUM);
	  
     /* initialize the buddy pool and tree */
     ret_val = buddy_init(MEM_SIZE);
     if (ret_val < 0) {
         printk(KERN_INFO "mem: could not allocate buddy pool\n");
         return -ENOMEM;
     }

     printk(KERN_INFO "mem: Module loaded successfully (device number: %d, pool size: %d)\n", MAJOR_NUM, MEM_SIZE);

   return ret_val;
}

/* Cleanup - unregister the appropriate file from /proc  */
void cleanup_module(void) { unregister_chrdev(MAJOR_NUM, DEVICE_NAME);  
printk(KERN_INFO "mem: Module unloaded successfully");}

MODULE_LICENSE("GPL");