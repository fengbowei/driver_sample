#ifndef _TEST_DEVICE_H_  
#define _TEST_DEVICE_H_  
  
#include <linux/cdev.h>  
#include <linux/semaphore.h>  
  
#define TEST_DEVICE_NODE_NAME  "test_device"  
#define TEST_DEVICE_FILE_NAME  "test_device"  
#define TEST_DEVICE_CLASS_NAME "test_device"  
  
struct test_device 
{  
    int m_val;  
    struct semaphore m_sem;  
    struct cdev m_cdev;  
};  
  
#endif 