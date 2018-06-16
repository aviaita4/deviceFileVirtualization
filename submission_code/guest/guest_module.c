#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
//#include <asm/uaccess.h>
#include<linux/syscalls.h>
#include<linux/init.h>
#include<linux/linkage.h>
#include<linux/cpumask.h>

extern long kern_kvm_hypercall0(unsigned int nr);
extern long kern_kvm_hypercall1(unsigned int nr, unsigned long p1);
extern long kern_kvm_hypercall2(unsigned int nr, unsigned long p1, unsigned long p2);
extern long kern_kvm_hypercall3(unsigned int nr, unsigned long p1, unsigned long p2, unsigned long p3);
extern long kern_kvm_hypercall4(unsigned int nr, unsigned long p1, unsigned long p2, unsigned long p3, unsigned long p4);

int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "guest_module"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80		/* Max length of the message from the device */

#define HOST_FILE_PATH "/dev/test_real_dev"
#define KVM_HYPERCALL_SUCCESS 0
#define DEVICE_OP_FAILURE -20
#define OPEN_HYPERCALL_NUM 100
#define RELEASE_HYPERCALL_NUM 200
#define READ_HYPERCALL_NUM 101
#define WRITE_HYPERCALL_NUM 102

#define DEVICE_NO_CONTACT "Did not invoke file operation on host device file at all!"

MODULE_LICENSE("GPL");

static int Major;		/* Major number assigned to our device driver */
//static int Device_Open = 0;	/* Is device open?  */

static struct file_operations fops = {
	.open = device_open,
	.read = device_read,
	.write = device_write,
	//.ioctl = device_ioctl
	//.mmap = device_mmap,
	.release = device_release
};

int init_module(void)
{
        Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
	  printk(KERN_ALERT "Registering test_dev device failed with %d\n", Major);
	  return Major;
	}
	
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);

	return SUCCESS;
}

void cleanup_module(void)
{
	unregister_chrdev(Major, DEVICE_NAME);
	//if (ret < 0)
	//	printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret);
}

//https://github.com/Refone/notes/blob/9a2cf6f0b3c522b160dc5e0e4ecc8ce26b050506/kvm-vmcall.md 

unsigned long mystrlen(char *p)
{
    unsigned long c=0;
    while(*p!='\0')
    {
        c++;
        *p++;
    }
    return(c);
}

static int device_open(struct inode *inode, struct file *file)
{

	//char __user *pathname = HOST_FILE_PATH;
	
	char* pathname = kmalloc(19, GFP_KERNEL);
	pathname = "/dev/test_real_dev";		

	unsigned long path_name_ptr = (unsigned long) pathname; 
	unsigned int flags = file->f_flags;
	int mode = (int)(file->f_mode);
	int path_size_sent = mystrlen(pathname);
	unsigned long ret = -1;

	printk(KERN_INFO "HOST_FILE_PATH = %s\n", pathname);	
	printk(KERN_INFO "mode = %d\n", mode);
	printk(KERN_INFO "flags = %d\n",flags);	
	printk(KERN_INFO "PATH_size = %ul\n", (unsigned long)path_size_sent);

	ret = kern_kvm_hypercall4(OPEN_HYPERCALL_NUM, path_name_ptr,(unsigned long) path_size_sent, (unsigned long)flags, (unsigned long)mode);

	if (ret!=KVM_HYPERCALL_SUCCESS){
		if(ret == DEVICE_OP_FAILURE){
                        printk(KERN_INFO DEVICE_NO_CONTACT);
                }
		printk(KERN_INFO "device could not be opened");
		return (int) ret;
	}

	printk(KERN_INFO "device file opened successfully");
	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
	unsigned long ret = kern_kvm_hypercall0(RELEASE_HYPERCALL_NUM);
	
	if (ret!=KVM_HYPERCALL_SUCCESS){
		if(ret == DEVICE_OP_FAILURE){
			printk(KERN_INFO DEVICE_NO_CONTACT);
		}
		printk(KERN_INFO "device could not be released");
		return (int) ret;
	}
	else{
		kfree(file);
		printk(KERN_INFO "device file released successfully");
	}
	return SUCCESS;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t * offset)
{
	long ret = kern_kvm_hypercall3(READ_HYPERCALL_NUM, (unsigned long)buffer, (unsigned long)length, (unsigned long)offset);

	if (ret!=KVM_HYPERCALL_SUCCESS){
		if(ret == DEVICE_OP_FAILURE){
                        printk(KERN_INFO DEVICE_NO_CONTACT);
                }
		printk(KERN_INFO "device could not be read");
		return (ssize_t) ret;
	}
	else{
		printk(KERN_INFO "device file read successfully");
	}
	return SUCCESS;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	long ret = kern_kvm_hypercall3(WRITE_HYPERCALL_NUM , (unsigned long)buff, (unsigned long)len, (unsigned long)off);

	if (ret!=KVM_HYPERCALL_SUCCESS){
		if(ret == DEVICE_OP_FAILURE){
                        printk(KERN_INFO DEVICE_NO_CONTACT);
                }
		printk(KERN_INFO "write did not work");
		return (ssize_t) ret;
	}
	else{
		printk(KERN_INFO "write worked successfully");
	}
	return SUCCESS;
}

