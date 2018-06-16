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
struct kvm* kvm;

int init_module(void);
void cleanup_module(void);
static int device_release(void);
static ssize_t device_read(unsigned long param1, unsigned long param2, unsigned long param3);
static ssize_t device_write(unsigned long param1, unsigned long param2, unsigned long param3);

unsigned long handle_hypercall(unsigned long nr, unsigned long a0, unsigned long a1, unsigned long a2, unsigned long a3);

extern unsigned long (*kvm_transfer_handle_module)(unsigned long nr, unsigned long a0, unsigned long a1, unsigned long a2, unsigned long a3);

static int device_open_normal(unsigned long param1, unsigned long param2, unsigned long param3, unsigned long param4);

/*
extern int get_unused_fd_flags(unsigned flags);
extern void put_unused_fd(unsigned int fd);
extern void fd_install(unsigned int fd, struct file *file);
*/

extern long ext_do_sys_open(int dfd, const char __user *filename, int flags, umode_t mode, struct file **filp);

//extern long do_sys_open(int dfd, const char __user *filename, int flags, umode_t mode);


int copy_from_guest(void* to, void* from, int num_bytes){

	//struct kvmgt_guest_info *info;
	//int idx, ret;
	//bool kthread = current->mm == NULL;

	//if (!handle_valid(handle))
	//	return -ESRCH;

	//info = (struct kvmgt_guest_info *)handle;
	//kvm = info->kvm;

	//if (kthread)
	//	use_mm(kvm->mm);

	//idx = srcu_read_lock(&kvm->srcu);
	return kvm_read_guest(kvm, gfn_to_gpa(from), to, num_bytes);
	//srcu_read_unlock(&kvm->srcu, idx);

	//if (kthread)
	//	unuse_mm(kvm->mm);

	//gpa_t guest_physical_addr
	//return kvm_read_guest(kvm_p, guest_physical_addr,  num_bytes);
}


int init_module(void)
{
	
	//handle = ?;
	struct kvmgt_guest_info *info;	

        host_file = (struct file*) kmalloc(sizeof(struct file), GFP_KERNEL);
	printk(KERN_INFO "Host module is up!\n");
	
	if (!handle_valid(handle))
		return -ESRCH;

	info = (struct kvmgt_guest_info *)handle;
	kvm = info->kvm;

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
			//return (unsigned long) device_open(a0, a1, a2, a3);
			return (unsigned long) device_open_normal(a0, a1, a2, a3);
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

static int device_open_normal(unsigned long param1, unsigned long param2, unsigned long param3, unsigned long param4)
{
        char __user *path_name = (char __user *) param1;
        unsigned long length_path = param2;

        //char* kern_path_name = "/home/shraavanth/DFV/deviceFileVirtualization/Host/new_file";
	char* kern_path_name = "/dev/test_real_dev";

	printk(KERN_INFO "direct access gives path: %s", path_name);
	char* recieved_device_path = kmalloc(length_path ,GFP_KERNEL);
//	int copy_size = copy_from_user(recieved_device_path, path_name, length_path);        
	printk(KERN_INFO "copy_from_user gives path: %s", recieved_device_path);
//	printk(KERN_INFO "copy_from_user return value: %d", copy_size);

	unsigned int flags = (unsigned int) param3 ;
        int  mode = (int) param4;

	mm_segment_t old_fs = get_fs();
	old_fs = get_fs();
	set_fs(KERNEL_DS);

	printk(KERN_INFO "FLAGS recieved = %d", flags);
	printk(KERN_INFO "mode recieved = %d", mode);
	

	long fd = ext_do_sys_open(AT_FDCWD, kern_path_name, flags, 0, &host_file);
	if (fd >= 0) {
		printk(KERN_INFO "FILE OPENED SUCCESSFULLY with fd = %ul", fd);
		
		if(host_file == NULL){
         	       printk(KERN_INFO "host_file ptr is NULL");
			
        	}else{
                	printk(KERN_INFO "host_file ptr is not null! !! !!.. trying more..");
			//printk(KERN_INFO "host_file ptr is NULL");
                        if(host_file->f_op == NULL){
                                printk(KERN_INFO "File operations still not available");
                        }else{
                                printk(KERN_INFO "File operations are available!!! YAY!!");
				//printk(KERN_INFO "TRYING TO OPEN AND CLOSE THE DEVICE FILE");

				
				//int rel_ret = host_file->f_op->release(host_file->f_inode, host_file);
                               // printk(KERN_INFO "Releasing Device file.... %d", rel_ret);
				//int open_ret = host_file->f_op->open(host_file->f_inode, host_file);
				//printk(KERN_INFO "Opening Device file.... %d", open_ret);
				//int close_ret = host_file->f_op->release(host_file->f_inode, host_file);               
                                //printk(KERN_INFO "Closing Device file.... %d", close_ret);
			}
        	}	


		sys_close(fd);
	}else{
		printk(KERN_INFO "COULD NOT OPEN FILE  with fd = %d", fd);
		return -EFAULT;
	}
	set_fs(old_fs);

	//return 0;
	return -EFAULT;

}

static int device_release(void)
{
	int ret_fop = host_file->f_op->release(host_file->f_inode, host_file);
	
	//kfree(host_file);
	//host_file = NULL;
	
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
