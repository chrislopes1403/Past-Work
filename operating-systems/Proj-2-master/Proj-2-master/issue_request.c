#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>

#include "systemcalls.h"

int (*STUB_issue_request)(int,int,int,int)=NULL;
EXPORT_SYMBOL(STUB_issue_request);
SYSCALL_DEFINE4(issue_request,int,num_pets,int,pet_type,int,start_floor,int,destination_floor)
{
	printk(KERN_NOTICE "Inside SYSCALL_DEFINE1 block. %s\n", __FUNCTION__);

	if (STUB_issue_request != NULL)
	{
		return STUB_issue_request(num_pets,pet_type,start_floor,destination_floor);
	}
	else
	{
		return -ENOSYS;
	}
}
