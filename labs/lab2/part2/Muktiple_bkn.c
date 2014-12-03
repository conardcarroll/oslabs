 #include <linux/module.h>
 #include <linux/kernel.h>
 #include <linux/kthread.h>
 #include <inux/semaphore.h>

struct task_struct *t1, *t2;
struct semaphore lock;


int incrementer(void *ptr) {
  printk(KERN_INFO "Consumer TID %d\n", (int)ptr);

  while (idx < 1000000) {
	//if(!down_interruotible(&lock)){
	if (idx >= loops) break;
	list[idx++]+= 1;
         schedule();
	if ((long)ptr == 1) csl++; else cs2++;
	up(&lock);
      }
   }
printk(KERN_INFO "Consumer %1d done\n" , (long)ptr);
return 0;
}

int init_module (void){
sema_init(&lock,1);
t1 = kthread_create(incrementer, (void*)id1, "inc1");
   t2 = kthread_create(incrementer, (void*)id2, "inc2");
   if (t1 && t2) {
      printk(KERN_INFO "Starting..\n");
      wake_up_process(t1);
      wake_up_process(t2);
   } else {
      printk(KERN_EMERG "Error\n");
   }

void cleanup_modlue(void)
