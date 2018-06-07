#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for put_user */


//#include <uapi/linux/kvm_para.h>
//#include<linux/kernel.h>
#include<linux/syscalls.h>
#include<linux/init.h>
#include<linux/linkage.h>
#include<linux/cpumask.h>



int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);


#define SUCCESS 0
#define DEVICE_NAME "test_dev"	/* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80		/* Max length of the message from the device */

#define HOST_FILE_PATH "/test.c"
#define KVM_HYPERCALL_SUCCESS 0
/* 
 * Global variables are declared as static, so are global within the file. 
 */

static int Major;		/* Major number assigned to our device driver */
static int Device_Open = 0;	/* Is device open?  
				 * Used to prevent multiple access to device */
static char msg[BUF_LEN];	/* The msg the device will give when asked */
static char *msg_Ptr;

static struct file_operations fops = {
	.open = device_open,
	.read = device_read,
	.write = device_write,
	.ioctl = device_ioctl,
	.mmap = device_mmap,
	.release = device_release
};

/*
 * This function is called when the module is loaded
 */
int init_module(void)
{
        Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
	  printk(KERN_ALERT "Registering test_dev device failed with %d\n", Major);
	  return Major;
	}

	printk(KERN_INFO "I was assigned major number %d. To talk to\n", Major);
	printk(KERN_INFO "the driver, create a dev file with\n");
	printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
	printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
	printk(KERN_INFO "the device file.\n");
	printk(KERN_INFO "Remove the device file and module when done.\n");

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
	int ret = 1;
	unregister_chrdev(Major, DEVICE_NAME);
	if (ret < 0)
		printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret);
}

/*
 * Methods
 */

/*
*hypercall1 for open and may be release too
*hypercall2 for read
*hypercall3 for write
*hypercall4 for mmap

*https://github.com/Refone/notes/blob/9a2cf6f0b3c522b160dc5e0e4ecc8ce26b050506/kvm-vmcall.md 
/* 
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *file)
{
	static int counter = 0;

	if (Device_Open)
		return -EBUSY;

	Device_Open++;
	sprintf(msg, "I already told you %d times Hello world!\n", counter++);
	msg_Ptr = msg;
	try_module_get(THIS_MODULE);

	return SUCCESS;


	// Implement 

	//push(path)
	//push(flags)
	//push(confg)
	
	//invoke hypercall

	//return SUCCESS;	

	char __user *pathname = HOST_FILE_PATH
	
	unsigned long path_name_ptr = (unsigned long) pathname; 
	unsigned int flags = file->f_flags;
	fmode_t mode = file->f_mode;
	
	long ret = kern_kvm_hypercall3( , path_name_ptr, (unsigned long)flags, (unsigned long)mode);
	//give hypercall numbers
	
	if (ret!=KVM_HYPERCALL_SUCCESS){
		printk(KERN_INFO "device could not be opened");
		return -EFAULT;
	}
	else{
		printk(KERN_INFO "device file opened successfully");
	}
	return SUCCESS;
}

/* 
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
	Device_Open--;		/* We're now ready for our next caller */

	/* 
	 * Decrement the usage count, or else once you opened the file, you'll
	 * never get get rid of the module. 
	 */
	module_put(THIS_MODULE);

	return 0;
	
	//Implement
	
	unsigned ret = kern_kvm_hypercall0();
	//add hypercall number
	//add these numbers in kernel
	if (ret!=KVM_HYPERCALL_SUCCESS){
		printk(KERN_INFO "device could not be released");
		return -EFAULT;
	}
	else{
		printk(KERN_INFO "device file released successfully");
	}
	return SUCCESS;
}

/* 
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	printk(KERN_ALERT "Sorry, this operation - read isn't supported.\n");
 
	printk(KERN_ALERT "Let me try invoking hypervisor and check functionalites!\n");

    return -EINVAL;	

    // Implement 

	//virtualToPhysical(buffer)
	//push(physicalAddress)
	//push(offset)
	//push(length)
	
	//invoke hypercall

    //pull(ssize)
	//return ssize;
	
	//unsigned long copy_from_user (void *to, const void __user *from, unsigned long n);
	//size is size of from place
	//copy_from_user mostly to be done in host. Send user data (file path, 
	//unsigned long kernelAdd;
	//if (copy_from_user(&kernelAdd, buffer, length) != 0)
        //	return -EFAULT;
	
	
	
	long ret = kern_kvm_hypercall3( , (unsigned long)buffer, (unsigned long)length, (unsigned long)offset);
	//give hypercall numbers
	//type cast??
	
	
	if (ret!=KVM_HYPERCALL_SUCCESS){
		printk(KERN_INFO "device could not be read");
		return -EFAULT;
	}
	else{
		printk(KERN_INFO "device file read successfully");
	}
	return SUCCESS;
	
}

/*  
 * Called when a process writes to dev file: echo "hi" > /dev/hello 
 */
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	printk(KERN_ALERT "Sorry, this operation - write isn't supported.\n");
	return -EINVAL;

	// Implement 

	//virtualToPhysical(buffer)
	//push(physicalAddress)
	//push(offset)
	//push(length)
	
	//invoke hypercall

    //pull(ssize)
	//return ssize;
	
	
	long ret = kern_kvm_hypercall3( , (unsigned long)buff, (unsigned long)len, (unsigned long)off);
	//give hypercall numbers
	//type cast??
	
	
	if (ret!=KVM_HYPERCALL_SUCCESS){
		printk(KERN_INFO "write did not work");
		return -EFAULT;
	}
	else{
		printk(KERN_INFO "write worked successfully");
	}
	return SUCCESS;
	
}
