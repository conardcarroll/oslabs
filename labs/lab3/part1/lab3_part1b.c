// use this delay 
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

struct workqueue_struct *queue;
struct delayed_work *dwork;
/* The argument to function is void* to allow any arrangement of input  *
 * parameters into the function.  In this example a string and two ints *
 * are passed to function through struct params                         */
 
void  function (struct work_struct *work)
{
	int cnt= 0;
	while (cnt++ < 10) 
	{
		printk(KERN_EMERG "Ding\n");
	}
}

int init_module(void) 
{
	queue = create_workqueue("queue");
	dwork = (struct delayed_work*)kmalloc(sizeof(struct delayed_work), GFP_KERNEL);
	INIT_DELAYED_WORK((struct delayed_work*)dwork, function);
	queue_delayed_work(queue, dwork, HZ);
	printk(KERN_EMERG "Module load successful\n");
	return 0;
}

void cleanup_module(void) {
	if(dwork && delayed_work_pending(dwork))
	{
		cancel_delayed_work(dwork);
	}
	flush_workqueue(queue);
	destroy_workqueue(queue);
	printk(KERN_EMERG "Module unload successful\n");
}

MODULE_LICENSE("GPL");


