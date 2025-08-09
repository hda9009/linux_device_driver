#ifndef _MY_PLATFORM_H_
#define _MY_PLATFORM_H_

#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/utsname.h>
#include <linux/slab.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE -1

#define RD_ONLY 0x01
#define WR_ONLY 0x10
#define RDWR 0x11

#define DEVICE_NAME "my_platform_device_1"
#define DEVICE_ID 1
#define DRV_DEVICE_NAME "my_platform_driver"
#define CLASS_NAME "my_platform_class"

#define MAX_DEVICES 1

int my_platform_driver_probe(struct platform_device *my_platform_device);
int my_platform_driver_remove(struct platform_device *my_platform_device);

loff_t pcd_llseek(struct file *filp, loff_t offset, int whence);
ssize_t pcd_read(struct file *filp, char __user *buf, size_t count, loff_t *position);
ssize_t pcd_write(struct file *filp, const char __user *buf, size_t count, loff_t *position);
int pcd_release(struct inode *inode, struct file *filp);
int pcd_open(struct inode *inode, struct file *filp);

struct pcdev_platform_data
{
    const char *device_serial_number; // Pointer to the device buffer
    size_t size;                      // Size of the device buffer
    int permissions;                  // Permissions for the device
};

struct platform_device_private_data
{
    char *buffer;        // Pointer to the device buffer
    dev_t device_number; // Device number for the device
    struct cdev cdev;    // for file operation
    struct pcdev_platform_data pdata;
};

struct platform_driver_private_data
{
    int total_devices;
    dev_t device_num_base; // initialized major number
    struct class *driver_class;
    struct device *driver_device;
    // struct platform_device *platform_device;
};

#endif /* _PLATFORM_H_ */