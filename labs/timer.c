#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");

static struct timer_list my_timer;
static int count;

void my_timer_callback( unsigned long data ) {
   printk(KERN_EMERG "my_timer_callback called (%ld).\n", jiffies );
   if (count++ < 10) mod_timer(&my_timer, jiffies + HZ);
}

int init_module( void ) {
   int ret;
   count = 1;

   printk("Timer module installing\n");

   // my_timer.function, my_timer.data
   setup_timer( &my_timer, my_timer_callback, 0 );

   printk( "Starting timer to fire in 1 second\n");
   ret = mod_timer( &my_timer, jiffies + HZ);
   if (ret) printk("Error in mod_timer\n");
   
   return 0;
}

void cleanup_module( void ) {
  int ret;

  ret = del_timer( &my_timer );
  if (ret) printk("The timer is a zombie...\n");

  printk("Timer module uninstalling\n");

  return;
}
