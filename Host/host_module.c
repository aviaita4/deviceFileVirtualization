#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include <linux/syscalls.h>
#include <linux/init.h>
#include <linux/linkage.h>
#include <linux/cpumask.h>

#define PATH_NAME_MAX_SIZE 100
#define FILE_OPERATION_SUCCESS 0

struct file* host_file;


int init_module(void);
void cleanup_module(void);


/*
 * This function is called when the module is loaded
 */
int init_module(void)
{
        
	printk(KERN_INFO "Host module is up!\n");

	return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
	/* 
	 * Unregister the device 
	 */
	printk(KERN_INFO "Host module is removed!\n");
}

/*
 * Methods
 */

 static int device_open(unsigned long param1, unsigned long param2, unsigned long param3)
 {

	//pull(path)
	//pull(flags)
	//pull(confg)

	//open file
	//get struct file 
	// store struct file

	//return SUCCESS;
	 
	 //unsigned long copy_from_user (void *to, const void __user *from, unsigned long n);
	//size is size of from place
	//copy_from_user mostly to be done in host. Send user data (file path, 
	//unsigned long kernelAdd;
	//if (copy_from_user(&kernelAdd, buffer, length) != 0)
        //	return -EFAULT;
	 
	
	 char __user *guest_path_name = (char __user *) param1; 
	 
	 char actual_path_name[PATH_NAME_MAX_SIZE];
	 
	 if (copy_from_user(&actual_path_name, buffer, PATH_NAME_MAX_SIZE) != 0)
        	return -EFAULT;
	 
	 unsigned int flags = (unsigned int) param2;
	 fmode_t mode = (fmode_t) param3;
	 
	 
	
 }

 static int device_read(unsigned long param1, unsigned long param2, unsigned long param3)
 {
	char __user *buffer = (char __user *)param1;
	size_t length = (size_t) param2;
	loff_t __user *offset = (loff_t __user *) param3;
	
	
	int ret_fop = host_file->fops->read(host_file, buffer, length, offset);
	
	if(ret_fop != FILE_OPERATION_SUCCESS){
		printk(KERN_INFO " Host: Read file operation successful");
		return -EFAULT;
	}
	else{
		printk(KERN_INFO " Host: Read file operation was unsuccessful");
	}
	
	
	if (copy_to_user(&, buffer, PATH_NAME_MAX_SIZE) != 0)
        	return -EFAULT;
	 
	
	return SUCCESS;
 }

 static int device_write(void)
 {

	//pull(physical address)
	//physicaltovirtualaddress

	//pull(offset)
	//pull(length)
	//file->fileop->write()

	//return SUCCESS;
 }

