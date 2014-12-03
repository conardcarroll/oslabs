// use this delay 
#include <linux/module.h>
#include <linux/kthread.h>

struct task_struct *ts;
int stop = 0;
int id = 101;
/* The argument to function is void* to allow any arrangement of input  *
 * parameters into the function.  In this example a string and two ints *
 * are passed to function through struct params                         */
int  function (void *data)
{
	int cnt= 0;
	while (!kthread_should_stop()&&(cnt++ < 10)) 
	{
		//int n = (*(int*)data);
		printk(KERN_EMERG "Ding");
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(HZ);
	}
	stop = 1;
	return 0;
}

int init_module(void) 
{
	printk(KERN_INFO "STARTING...");
	ts = kthread_run(function, (void*)&id, "spawn");
	return 0;
}

void cleanup_module(void) 
{
	if(stop == 0)
	{
		kthread_stop(ts);
   	}
	printk(KERN_EMERG "Module unload successful\n");
}

MODULE_LICENSE("GPL");


