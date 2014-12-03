 #include <linux/module.h>
 #include <linux/kernel.h>
 #include <linux/kthread.h>
 #include <linux/semaphore.h>

struct task_struct *t1, *t2;
struct semaphore lock;

int arr[1000000];
int stat[1000000];
int idx = 0;
int cs1 = 0;
int cs2 = 0;
int id1 = 1, id2 = 2;
int stop1 = 0, stop2 = 0;

int incrementer(void *ptr) {
  	printk(KERN_INFO "Consumer TID %d\n", (int)ptr);

 	while (idx < 1000000) {
	//if(!down_interruotible(&lock)){
	//if (idx >= loops) break;
		arr[idx++]+= 1;
		schedule();
		if ((int)ptr == 1) {
			cs1++;
		} else { 
			cs2++;
		}
	//up(&lock);
	//}
	}
	printk(KERN_INFO "Consumer %1d done\n" , (int)ptr);
	if ((int)ptr == 1) {
			stop1 = 1;
		} else { 
			stop2 = 1;
		}
	return 0;
}

int init_module (void){
	//sema_init(&lock,1);
	t1 = kthread_create(incrementer, (void*)id1, "inc1");
   	t2 = kthread_create(incrementer, (void*)id2, "inc2");
   	if (t1 && t2) {
      		printk(KERN_INFO "Starting..\n");
      		wake_up_process(t1);
      		wake_up_process(t2);
   	} else {
      		printk(KERN_EMERG "Error\n");
   	}
	return 0;
}

void cleanup_module(void) {
	int i = 0;
	for(; i < 100000; i++) {
		if(arr[i] > 0) {
			stat[arr[i]] += 1;
			//printk(KERN_INFO "stat %d is %d", arr[i], stat[arr[i]]);
		}
	}
	int j = 0;
	for(; j < 100000; j++) {
		if(stat[j] > 0) {
			printk(KERN_INFO "stat %d is %d", j, stat[j]);
		}
	}
	printk(KERN_INFO "cs1 %d cs2 %d cs1+cs2 %d", cs1, cs2, cs1 + cs2);
	if(stop1 == 0) {
		kthread_stop(t1);
	}

	printk(KERN_EMERG "t1 stop");

	if(stop2 == 0) {
		kthread_stop(t2);
	}
	printk(KERN_EMERG "module unload");
}



MODULE_LICENSE("GPL");


