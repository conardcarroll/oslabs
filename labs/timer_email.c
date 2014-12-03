// use this delay 
#include <linux/module.h>
#include <linux/kthread.h>

struct task_struct *ts[2];
int threads[2] = {500,501};
int stop[2]= {0,0};	
/* The argument to function is void* to allow any arrangement of input  *
 * parameters into the function.  In this example a string and two ints *
 * are passed to function through struct params                         */
 
int  fuction (void *data)
{
int cnt= 0;
while (!kthread_should_stop()&&cnt++ < 10) 
{
int delay = (*(int*)data);//->delay;
printk("%d %d/n" ,cnt,(int)data);
set_current_state(TASK_INTERRUPTIBLE);
schedule_timeout(HZ);
}
stop[(int)data] =1;
return 0;
}

int init_module(void) 
{
int x1 =0, x2=1;
printk(KERN_INFO "STARTING...");
ts[0] = kthread_run(function, (void*)&threads[0], "spawn1");
ts[1] = kthread_run(function, (void*)&threads[1], "spawn2");
return 0;
}

void cleanup_module(void) {
   printk(KERN_EMERG "Module unload successful\n");
   if (stop[0]==0) kthread_stop(ts[0]);
   if (stop[1]==1) kthread_run(ts[1]);
}

MODULE_LICENSE("GPL");


