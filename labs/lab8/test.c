/*
 *  ioctl.c - the process to use ioctl's to control the kernel module
 *
 *  Every device can have its own ioctl commands to send information 
 *  from a process to the kernel, write information to a process, both 
 *  or neither.  The ioctl function is called with three parameters: 
 *    the file descriptor of the appropriate device file, 
 *    the ioctl number, 
 *    and an open parameter of type long
 *
 *  The ioctl number encodes the major device number, the type of the ioctl, 
 *  the command, and the type of the parameter.  The ioctl number is usually 
 *  created by a macro call (_IO, _IOR, _IOW or _IOWR --- depending on the 
 *  type) in a header file.  This header file should then be included both 
 *  by the programs which will use ioctl and by the kernel module.  The
 *  header file is 'sys/ioctl.h'
 *
 *  It is best to receive an official ioctl assignment, so if you accidentally 
 *  get somebody else's ioctls, or if they get yours, you'll know something is 
 *  wrong.
 *
 *  Until now we could have used cat for input and output.  But now
 *  we need to do ioctl's, which require writing our own process.
 *
 *  Note: ioctl's read is to send information to the kernel and its write 
 *  is to receive information from the kernel.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
/* 
 * device specifics, such as ioctl numbers and the major device file. 
 */
#include "memdev.h"


int get_mem(int mem, int size) {return ioctl(mem, IOCTL_GET_MEM, size);}
int free_mem(int mem, int ref) {return ioctl(mem, IOCTL_FREE_MEM, ref);}
int write_mem(int mem, int ref, char* buf)
{
	printf("write_mem mem %d ref %d buf %s len %d\n", mem, ref, buf, strlen(buf));
	int ret = 0;
	ret = ioctl(mem, IOCTL_SET_IDX, ref);
	printf("set index %d\n", ret);
	
	// ret = write(mem, buf, strlen(buf));
	ret = ioctl(mem, IOCTL_WRITE_MEM, buf);
	printf("wrote %d\n", ret);
	// write(mem, buf, NULL, NULL);
	
	return ret	;
}
int read_mem(int mem, int ref, char* buf, int size)
{
	int ret = 0;
	ret = ioctl(mem, IOCTL_SET_IDX, ref);
	printf("set index %d\n", ret);
	
	ret = ioctl(mem, IOCTL_READ_SIZE, size);
	printf("set read size %d\n", ret);
	
	ret = ioctl(mem, IOCTL_READ_MEM, buf);
	printf("read %d bytes\n", ret);
	return ret;
	
}


/* Main - Call the ioctl functions */
main() {
   int mem, ret_val;
   char buffer[4096];
   int ref;
   char *msg0 = " ";
   char *msg1 = "Put that in your pipe and smoke it";
   char *msg2 = "Try this on for size";
   
   mem = open("/dev/mem_dev", 0);
   if (mem < 0) {
      printf("Can't open device file: /dev/mem_dev%s\n");
      exit(-1);
   }
   
   ref = get_mem(mem, 100);
   sprintf(buffer, "Hello buddy");
   write_mem(mem, ref, buffer);
   read_mem(mem, ref+3, buffer, 10);
   printf("buffer: %s\n", buffer);  /* buffer: lo buddy */
   free_mem(mem, ref);
   return close(mem);
   
      //
   // ioctl_set_msg(file_desc, msg0);
   // ioctl_get_msg(file_desc);
   // ioctl_set_msg(file_desc, msg1);
   // ioctl_get_msg(file_desc);
   // ioctl_set_msg(file_desc, msg2);
   // ioctl_get_msg(file_desc);
   // ioctl_get_nth_byte(file_desc);
   //
   // close(file_desc);
}