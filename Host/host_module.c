#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include <linux/syscalls.h>
#include <linux/init.h>
#include <linux/linkage.h>
#include <linux/cpumask.h>


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

 static int device_open(void)
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
	
 }

 static int device_read(void)
 {

	//pull(physical address)
	//physicaltovirtualaddress

	//pull(offset)
	//pull(length)
	//file->fileop->read()

	//return SUCCESS;
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

