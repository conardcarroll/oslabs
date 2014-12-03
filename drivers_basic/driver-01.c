#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/semaphore.h>

/* Sample device driver

   The device is just an array that can hold a maximum of 100 characters 
   and its concurrent access can be controlled using the semaphore "sem".

   The following 4 operations for the device will be implemented:
      Open, Read, Write, Close.
*/
struct device {
   char array[100];
   struct semaphore sem;
} char_arr;
