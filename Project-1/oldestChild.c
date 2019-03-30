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

int processID = 0;
module_param(processID,int,0);

void print_details(struct task_struct *t);
void oldestchild(struct task_struct *t);

static int custom_module_init(void) {
		printk(KERN_INFO "Loading Module\n");
		printk(KERN_INFO "Root processID:%d\n",processID);
		oldestchild(current);
		return 0;
}

static void custom_module_exit(void) {
	printk(KERN_INFO "Removing Module...\n");
}

void oldestchild(struct task_struct *t) {
	//If no process ID is provided or the process ID is invalid, print an error message to kernel log.
		struct task_struct *task = t;
		struct task_struct *temp_task;

    for_each_process(task) {
		if (task->pid == processID){
			printk(KERN_INFO "PID: [%d]\n", task->pid);
			printk(KERN_INFO "Parent PID: [%d]\n", (task->parent)->pid);
			printk(KERN_INFO "Process Start time: [%d]\n", task->start_time);
			printk(KERN_INFO "Executable Name: [%s]",task->comm);

			list_for_each(list, task->children) {
				temp_task = list_entry(list, struct task_struct, sibling);
				printk(KERN_INFO "\tChild PID: [%d]\n",temp_task->pid);
				printk(KERN_INFO "\tChild Start time: [%d]\n", temp_task->start_time);
				printk(KERN_INFO "\tChild Executable Name: [%s]\n",temp_task->comm);
			}

		}
	}
}

module_init(custom_module_init);
module_exit(custom_module_exit);