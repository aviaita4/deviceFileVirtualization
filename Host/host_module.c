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
#define PATH_NAME_MAX_SIZE 20
#define FILE_OPERATION_SUCCESS 0
#define DEVICE_OP_FAILURE -20

#define OPEN_HYPERCALL_NUM 100
#define RELEASE_HYPERCALL_NUM 200
#define READ_HYPERCALL_NUM 101
#define WRITE_HYPERCALL_NUM 102

struct file* host_file;


int init_module(void);
void cleanup_module(void);
void file_open(struct file *filp, const char *path, int flags, int rights);
static int device_open(unsigned long param1, unsigned long param2, unsigned long param3);
static int device_release(void);
static ssize_t device_read(unsigned long param1, unsigned long param2, unsigned long param3);
static ssize_t device_write(unsigned long param1, unsigned long param2, unsigned long param3);

unsigned long handle_hypercall(unsigned long nr, unsigned long a0, unsigned long a1, unsigned long a2, unsigned long a3);

extern unsigned long (*kvm_transfer_handle_module)(unsigned long nr, unsigned long a0, unsigned long a1, unsigned long a2, unsigned long a3);

//extern unsigned long (*kvm_transfer_handle_module);

int init_module(void)
{
        host_file = NULL;
	printk(KERN_INFO "Host module is up!\n");
	
	// Link hypercall handler
	kvm_transfer_handle_module = &handle_hypercall;	

	return SUCCESS;
}

void cleanup_module(void)
{
	printk(KERN_INFO "Host module is removed!\n");
}

unsigned long handle_hypercall(unsigned long nr, unsigned long a0, unsigned long a1, unsigned long a2, unsigned long a3)
{
	unsigned long ret = DEVICE_OP_FAILURE;
	printk(KERN_INFO "Hypercall handler module invoked finally!");
	printk(KERN_INFO "Hypercall recieved for number = %d", nr);

	switch(nr){
	
		case OPEN_HYPERCALL_NUM:
			return (unsigned long) device_open(a0, a1, a2);
		case RELEASE_HYPERCALL_NUM: 
			return (unsigned long) device_release();
		case READ_HYPERCALL_NUM:
			return (unsigned long) device_read(a0, a1, a2);
		case WRITE_HYPERCALL_NUM:
			return (unsigned long) device_write(a0, a1, a2);
		default:
			;
	}

	return ret;
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

void open_filep(){
	/*
	int fd = get_unused_fd_flags(flags);

	if (fd >= 0) {
		struct open_flags op;
		int lookup = build_open_flags(flags, mode, &op);
		struct file *f = do_filp_open(dfd, filename, &op, lookup);
		if (IS_ERR(f)) {
			put_unused_fd(fd);
			fd = PTR_ERR(f);
		} else {
			fsnotify_open(f);
			fd_install(fd, f);
			trace_do_sys_open((char *) filename, flags, mode);
		}

		*_f = f;
	}

	return fd;

	*/
}

int mimic_do_sys_open(int dfd, const char __user *filename, int flags, umode_t mode, struct file ** f){

	struct open_flags op;
	int fd = build_open_flags(flags, mode, &op);
	struct filename *tmp;

	if (fd)
		return fd;

	tmp = getname(filename);
	if (IS_ERR(tmp))
		return PTR_ERR(tmp);

	fd = get_unused_fd_flags(flags);
	if (fd >= 0) {
		struct file *f = do_filp_open(dfd, tmp, &op);
		if (IS_ERR(f)) {
			put_unused_fd(fd);
			fd = PTR_ERR(f);
		} else {
			fsnotify_open(f);
			fd_install(fd, f);
		}
		*f = f;
	}
	putname(tmp);
	return fd;
}

static int device_open(unsigned long param1, unsigned long param2, unsigned long param3)
{
	printk(KERN_INFO "reached here: device open");

	char __user *path_name = (char __user *) param1; 
	char kern_path_name[PATH_NAME_MAX_SIZE];
	unsigned int flags = (unsigned int) param2 ;
	int  mode = (int) param3;

	printk(KERN_INFO "reached here %s", path_name);	
	
	 if (copy_from_user(kern_path_name, path_name, PATH_NAME_MAX_SIZE) != 0){
		printk(KERN_INFO "copying file name failed");		
		return -EFAULT;
	}
	printk(KERN_INFO "reached here %s", kern_path_name);

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
	ssize_t ret_fop = DEVICE_OP_FAILURE;
	char __user *buffer = (char __user *)param1;
	size_t length = (size_t) param2;
	char kern_buffer[length];

	loff_t __user *offset = (loff_t __user *) param3;
	loff_t kern_offset;

	if (copy_from_user(&kern_offset, offset, sizeof(loff_t)) != 0)
		return -EFAULT;

	ret_fop = host_file->f_op->read(host_file, kern_buffer, length, &kern_offset);

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
	ssize_t ret_fop = DEVICE_OP_FAILURE;	
	char __user *buffer = (char __user *)param1;
	size_t length = (size_t) param2;
	char kern_buffer[length];

	loff_t __user *offset = (loff_t __user *) param3;
	loff_t kern_offset;

	if (copy_from_user(&kern_offset, offset, sizeof(loff_t)) != 0)
		return -EFAULT;

	if (copy_from_user(kern_buffer, buffer, length) != 0)
		return -EFAULT;

	ret_fop = host_file->f_op->write(host_file, kern_buffer, length, &kern_offset);

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

MODULE_LICENSE("GPL");
