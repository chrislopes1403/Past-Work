#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/mutex.h>
//#include <semaphore.h>
//#include <asm-generic/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Thread example illustrating different blocking commands");

#define ENTRY_NAME "thread_example"
#define ENTRY_SIZE 200
#define PERMS 0644
#define PARENT NULL
//GLOBALS
static struct file_operations fops;
#define BUFFER_SIZE 20
#define KTHREAD_STRING_1 "kthread 1"
#define KTHREAD_STRING_2 "kthread_2"

static struct task_struct *kthread1;
static struct task_struct *kthread2;


static  struct mutex shared_data_mutex;
static char  *shared_data1;

int test_data1=0;
int threadcount=0;

//char *boo1="kthread1";
//char *boo2="kthread2";
sem_t sem_mutex;

//static int counter;
static char *message;
static int read_p;
/******************************************************************************/

int my_run(void*data) {
	char *name=(char*)data;
	
	while (!kthread_should_stop()){
		//sem_wait(&sem_mutex);
		test_data1++;
		strcpy(shared_data1,name);
		strcat(shared_data1," has updated the data ");
		mutex_unlock(&shared_data_mutex);
		ssleep(1);
		//sem_post(&sem_mutex);
		schedule();
		mutex_lock_interruptible(&shared_data_mutex);
	}
	mutex_unlock(&shared_data_mutex);
	//sem_post(&sem_mutex);
	printk("The %s has terminated\n",name);
	return 0;
}
/******************************************************************************/

int thread_proc_open(struct inode *sp_inode, struct file *sp_file) {
	read_p = 1;

	message = kmalloc(sizeof(char) * ENTRY_SIZE, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
	if (message == NULL) {
		printk(KERN_WARNING "hello_proc_open");
		return -ENOMEM;
	}
	//sem_wait(&sem_mutex);
	mutex_lock_interruptible(&shared_data_mutex);
	strcpy(message,shared_data1);
	sprintf(message," The count is know %d\n",test_data1);
	strcat(message,shared_data1);
	mutex_unlock(&shared_data_mutex);
	//sem_post(&sem_mutex);
	return 0;
}

ssize_t thread_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset) {
	int len = strlen(message);
	
	read_p = !read_p;
	if (read_p)
		return 0;
	printk("proc called read\n");
	copy_to_user(buf, message, len);
	return len;
}

int thread_proc_release(struct inode *sp_inode, struct file *sp_file) {
	printk("proc called release\n");
	kfree(message);
	return 0;
}

/******************************************************************************/
static int compete_init(void) {
printk("/proc/%s create\n",ENTRY_NAME);

mutex_init(&shared_data_mutex);

shared_data1 = kmalloc(sizeof(char)* BUFFER_SIZE,__GFP_RECLAIM|__GFP_IO|__GFP_FS);

kthread1 = kthread_run(my_run,(void*)KTHREAD_STRING_1,KTHREAD_STRING_1);
if(IS_ERR(kthread1)){
printk("ERROR! kthread_run1\n");
return PTR_ERR(kthread1);
}

kthread2=kthread_run(my_run,(void*)KTHREAD_STRING_2,KTHREAD_STRING_2);
if(IS_ERR(kthread2)){
printk("ERROR! kthread_run2\n");
return PTR_ERR(kthread2);
}

 fops.open = thread_proc_open;
 fops.read = thread_proc_read;
 fops.release = thread_proc_release;

if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops)) {
                printk("ERROR! proc_create\n");
                remove_proc_entry(ENTRY_NAME, NULL);
                return -ENOMEM;
        }



return 0;
}

/*
static int counter_init(void) {
	fops.open = thread_proc_open;
	fops.read = thread_proc_read;
	fops.release = thread_proc_release;


	
	if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops)) {
		printk(KERN_WARNING "thread_init");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	}

	if (IS_ERR(thread1.kthread)) {
		printk(KERN_WARNING "error spawning thread");
		remove_proc_entry(ENTRY_NAME, NULL);
		return PTR_ERR(thread1.kthread);
	}
	
	return 0;
}
*/

module_init(compete_init);

static void thread_exit(void) {

int ret = kthread_stop(kthread1);

if(ret!=-EINTR){
printk("%s has stooped\n",KTHREAD_STRING_1);
}

ret = kthread_stop(kthread2);

if(ret!=-EINTR){
printk("%s has stopped\n",KTHREAD_STRING_2);
}

kfree(shared_data1);

remove_proc_entry(ENTRY_NAME, NULL);
printk(KERN_NOTICE "Removing /proc/%s\n", ENTRY_NAME);



}
module_exit(thread_exit);
