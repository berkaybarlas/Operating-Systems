/*
COMP 304 Spring 2019 Project-1
KUSIS ID: 60210 PARTNER NAME: Ege Onat Özsüer
KUSIS ID: 54512 PARTNER NAME: Berkay Barlas

 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Berkay Barlas, Ege Onat Ozsuer");

int processID = 0, processPrio = 0;
module_param(processID,int,0);
module_param(processPrio,int,0);

void print_details(struct task_struct *t);

static int custom_module_init(void){
		printk(KERN_INFO "Loading Module\n");
		printk(KERN_INFO "%d %d\n",processID,processPrio);
		print_details(current);
		return 0;
}

static void custom_module_exit(void) {
	printk(KERN_INFO "Removing Module...\n");
}

void print_details(struct task_struct *t){

}

module_init(custom_module_init);
module_exit(custom_module_exit);