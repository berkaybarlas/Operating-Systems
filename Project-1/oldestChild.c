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
#include <linux/sched/signal.h>
#include <linux/pid.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Berkay Barlas, Ege Onat Ozsuer");

int processID = 0;
module_param(processID,int,0);

void print_details(struct task_struct *t);
void oldestchild(struct task_struct *t);

static int custom_module_init(void) {
		printk(KERN_INFO "Loading Module\n");
		printk(KERN_INFO "Root processID:%d\n",processID);
		struct pid *pid_struct;
		struct task_struct *task;
		
		pid_struct = find_get_pid(processID);
		task = pid_task(pid_struct, PIDTYPE_PID);

		//If no process ID is provided or the process ID is invalid, print an error message to kernel log.

		printk(KERN_INFO "PID: [%d]\n", task->pid);
		printk(KERN_INFO "Parent PID: [%d]\n", (task->parent)->pid);
		printk(KERN_INFO "Process Start time: [%d]\n", task->start_time);
		printk(KERN_INFO "Executable Name: [%s]",task->comm);
		oldestchild(task);
		
		return 0;
}

static void custom_module_exit(void) {
	printk(KERN_INFO "Removing Module...\n");
}

void oldestchild(struct task_struct *t) {
		struct task_struct *task = t;
		struct task_struct *oldestProcess = NULL;
		struct task_struct *temp_task;
		struct list_head *list;
	
	//prints the PIDs and executable names of the oldest children
	long oldestTime = NULL; 
	list_for_each(list, &task->children) {
		temp_task = list_entry(list, struct task_struct, sibling);
		if(oldestTime == NULL || oldestTime > temp_task->start_time) {
			oldestProcess = temp_task;
			oldestTime = temp_task->start_time;
		}
		oldestchild(temp_task);
	}
	if(*oldestProcess != NULL) {
		printk(KERN_INFO "\tOldest Child of [%d] PID: [%d]\n",(temp_task->parent)->pid,temp_task->pid);
		printk(KERN_INFO "\tOldest Child of [%d] Start time: [%d]\n",(temp_task->parent)->pid ,temp_task->start_time);
		printk(KERN_INFO "\tOldest Child of [%d] Executable Name: [%s]\n",(temp_task->parent)->pid ,temp_task->comm);
	}
}

module_init(custom_module_init);
module_exit(custom_module_exit);
