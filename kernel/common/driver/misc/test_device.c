#include <linux/init.h>  
#include <linux/module.h>  
#include <linux/types.h>  
#include <linux/fs.h>  
#include <linux/proc_fs.h>  
#include <linux/device.h>  
#include <asm/uaccess.h> 
#include <linux/slab.h>


#include "test_device.h"

static dev_t dev_no = 0;
struct test_device* test_device_data = NULL;
struct class* test_driver_class = NULL;
struct device* test_device_dev = NULL;


static int test_driver_open(struct inode* inode, struct file* filp);  
static int test_driver_release(struct inode* inode, struct file* filp);  
static ssize_t test_driver_read(struct file* filp, char __user *buf, size_t count, loff_t* f_pos);  
static ssize_t test_driver_write(struct file* filp, const char __user *buf, size_t count, loff_t* f_pos);


static struct file_operations test_driver_fops = 
{  
    .owner = THIS_MODULE,  
    .open = test_driver_open,  
    .release = test_driver_release,  
    .read = test_driver_read,  
    .write = test_driver_write,   
};


static int test_driver_open(struct inode* inode, struct file* filp) 
{  
    struct test_device* test_device_data;          
    test_device_data = container_of(inode->i_cdev, struct test_device, m_cdev);  
    filp->private_data = test_device_data;  
      
    return 0;  
}  
  
static int test_driver_release(struct inode* inode, struct file* filp) 
{  
    return 0;  
}  
  

static ssize_t test_driver_read(struct file* filp, char __user *buf, size_t count, loff_t* f_pos) 
{  
    ssize_t err = 0;  
    struct test_device* test_device_data = filp->private_data;          
  
    if(down_interruptible(&(test_device_data->m_sem))) 
    {  
        return -ERESTARTSYS;  
    }  
  
    if(count < sizeof(test_device_data->m_val)) 
    {  
        goto out;  
    }          

    if(copy_to_user(buf, &(test_device_data->m_val), sizeof(test_device_data->m_val))) 
    {  
        err = -EFAULT;  
        goto out;  
    }  
  
    err = sizeof(test_device_data->m_val);  
  
out:  
    up(&(test_device_data->m_sem));  
    return err;  
}  
  
static ssize_t test_driver_write(struct file* filp, const char __user *buf, size_t count, loff_t* f_pos) {  
    struct test_device* test_device_data = filp->private_data;  
    ssize_t err = 0;          
  
    if(down_interruptible(&(test_device_data->m_sem))) {  
        return -ERESTARTSYS;          
    }          
  
    if(count != sizeof(test_device_data->m_val)) {  
        goto out;          
    }          
 
    if(copy_from_user(&(test_device_data->m_val), buf, count)) {  
        err = -EFAULT;  
        goto out;  
    }  
  
    err = sizeof(test_device_data->m_val);  
  
out:  
    up(&(test_device_data->m_sem));  
    return err;  
}

static int __init test_driver_init(void)
{   
    int err = -1;
  
    printk(KERN_ALERT"Initializing test device.\n");          

    err = alloc_chrdev_region(&dev_no, 0, 1, TEST_DEVICE_NODE_NAME);  
    if(err < 0) 
    {  
        printk(KERN_ALERT"Failed to alloc char dev region.\n");  
        goto fail;  
    }          
  
    test_device_data = kmalloc(sizeof(struct test_device), GFP_KERNEL);  
    if(!test_device_data) 
    {  
        err = -ENOMEM;  
        printk(KERN_ALERT"Failed to alloc test_device_data.\n");  
        goto unregister;  
    }   
    memset(test_device_data, 0, sizeof(struct test_device));  

    cdev_init(&(test_device_data->m_cdev), &test_driver_fops);          
  
    err = cdev_add(&(test_device_data->m_cdev), dev_no, 1);  
    if(err) 
    {  
        printk(KERN_ALERT"Failed to setup dev: %d.\n", err);  
        goto cleanup; 
    }          
  
    sema_init(&(test_device_data->m_sem), 1);  
    test_device_data->m_val = 0;
  
    // create class under /sys/class/ 
    test_driver_class = class_create(THIS_MODULE, TEST_DEVICE_CLASS_NAME);  
    if(IS_ERR(test_driver_class)) 
    {  
        err = PTR_ERR(test_driver_class);  
        printk(KERN_ALERT"Failed to create hello class.\n");  
        goto destroy_cdev;  
    }

    // create test_device under /dev
    test_device_dev = device_create(test_driver_class, NULL, dev_no, "%s", TEST_DEVICE_FILE_NAME);  
    if(IS_ERR(test_device_dev))
    {  
        err = PTR_ERR(test_device_dev);  
        printk(KERN_ALERT"Failed to create hello device.");  
        goto destroy_class;  
    }          

  
    printk(KERN_ALERT"Succedded to initialize hello device.\n");  
    return 0;
  
destroy_class:  
    class_destroy(test_driver_class);  
  
destroy_cdev:  
    cdev_del(&(test_device_data->m_cdev));  
  
cleanup:  
    kfree(test_device_data);  
  
unregister:  
    unregister_chrdev_region(dev_no, 1);  
  
fail:  
    return err;  
}  
  

static void __exit test_driver_exit(void) 
{
    printk(KERN_ALERT"Destroy hello device.\n");          
        
  

    device_destroy(test_driver_class, dev_no);
    class_destroy(test_driver_class);           
   
    if(test_device_data) 
    {  
        cdev_del(&(test_device_data->m_cdev));  
        kfree(test_device_data);  
    }          
  
    //release dev no
    unregister_chrdev_region(dev_no, 1);  
}  
  
MODULE_LICENSE("HelloWorld");  
MODULE_DESCRIPTION("Test Device Driver");  
  
module_init(test_driver_init);  
module_exit(test_driver_exit);