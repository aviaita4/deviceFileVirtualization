#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include <linux/syscalls.h>
#include <linux/init.h>
#include <linux/linkage.h>
#include <linux/cpumask.h>

// File system open etc.,
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>

#define SUCCESS 0
#define PATH_NAME_MAX_SIZE 100
#define FILE_OPERATION_SUCCESS 0

struct file* host_file;


int init_module(void);
void cleanup_module(void);
void file_open(struct file *filp, const char *path, int flags, int rights);
static int device_open(unsigned long param1, unsigned long param2, unsigned long param3);
static int device_release(void);
static ssize_t device_read(unsigned long param1, unsigned long param2, unsigned long param3);
static ssize_t device_write(unsigned long param1, unsigned long param2, unsigned long param3);

int init_module(void)
{
        host_file = NULL;
	printk(KERN_INFO "Host module is up!\n");
	return SUCCESS;
}

void cleanup_module(void)
{
	printk(KERN_INFO "Host module is removed!\n");
}

void file_open(struct file *filp, const char *path, int flags, int rights) 
{
    mm_segment_t oldfs;
    int err = 0;

    oldfs = get_fs();
    set_fs(get_ds());
    filp = filp_open(path, flags, rights);
    set_fs(oldfs);
    if (IS_ERR(filp)) {
	filp = NULL;
        return;
    }
    return;
}

static int device_open(unsigned long param1, unsigned long param2, unsigned long param3)
{
	 char __user *path_name = (char __user *) param1; 
	 char kern_path_name[PATH_NAME_MAX_SIZE];
	
	 unsigned int flags = (unsigned int) param2 ;
	 
	 fmode_t mode = (fmode_t) param3;

	 if (copy_from_user(&kern_path_name, path_name, PATH_NAME_MAX_SIZE) != 0)
		return -EFAULT;

	// Open file
	
	// Get file*
	
	// Store in host_file
	file_open(host_file, kern_path_name, flags, mode);
	
	return SUCCESS; 
}

static int device_release(void)
{
	int ret_fop = host_file->f_op->release(host_file->f_inode, host_file);
	
	kfree(host_file);
	host_file = NULL;
	
	return ret_fop;
}

static ssize_t device_read(unsigned long param1, unsigned long param2, unsigned long param3)
{
	char __user *buffer = (char __user *)param1;
	char kern_buffer[length];

	size_t length = (size_t) param2;

	loff_t __user *offset = (loff_t __user *) param3;
	loff_t kern_offset;

	if (copy_from_user(&kern_offset, offset, sizeof(loff_t)) != 0)
		return -EFAULT;

	int ret_fop = host_file->f_op->read(host_file, kern_buffer, length, kern_offset);

	if(ret_fop < 0){
		printk(KERN_INFO " Host: Read file operation successful");
		return ret_fop;
	}
	else{
		printk(KERN_INFO " Host: Read file operation was unsuccessful");
	}

	if ((copy_to_user(buffer, kern_buffer, length) != 0) && (copy_to_user(offset, &kern_offset, sizeof(loff_t)) != 0))
		return ret_fop;

	return ret_fop;
}

static ssize_t device_write(unsigned long param1, unsigned long param2, unsigned long param3)
{
	char __user *buffer = (char __user *)param1;
	char kern_buffer[length];

	size_t length = (size_t) param2;

	loff_t __user *offset = (loff_t __user *) param3;
	loff_t kern_offset;

	if (copy_from_user(&kern_offset, offset, sizeof(loff_t)) != 0)
		return -EFAULT;

	if (copy_from_user(kern_buffer, buffer, length)) != 0)
		return -EFAULT;

	ssize_t ret_fop = host_file->f_op->write(host_file, kern_buffer, length, kern_offset);

	if(ret_fop < 0){
		printk(KERN_INFO " Host: Write file operation successful");
		return ret_fop;
	}
	else{
		printk(KERN_INFO " Host: Write file operation was unsuccessful");
	}

	if (copy_to_user(offset, &kern_offset, sizeof(loff_t)) != 0)
		return -EFAULT;

	return ret_fop;
}

