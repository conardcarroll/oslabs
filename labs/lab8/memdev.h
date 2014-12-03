/*
 *  memdev.h - the header file with the ioctl definitions.
 *
 *  The declarations here have to be in a header file, because
 *  they need to be known both to the kernel module
 *  (in chardev.c) and the process calling ioctl (ioctl.c)
 */

#ifndef MEMDEV_H
#define MEMDEV_H

#include <linux/ioctl.h>

#define MEM_SIZE (1 << 12)
// #define MEM_SIZE 16777216
#define MAJOR_NUM 111
#define IOCTL_GET_MEM _IOR(MAJOR_NUM, 0, int)
#define IOCTL_FREE_MEM _IOR(MAJOR_NUM, 1, int)
#define IOCTL_WRITE_MEM _IOR(MAJOR_NUM, 2, char*)
#define IOCTL_READ_MEM _IOR(MAJOR_NUM, 3, char*)
#define IOCTL_SET_IDX _IOR(MAJOR_NUM, 4, int)
#define IOCTL_READ_SIZE _IOR(MAJOR_NUM, 5, int)

/* 
 * The name of the device file 
 */
#define DEVICE_NAME "mem_dev"


#endif