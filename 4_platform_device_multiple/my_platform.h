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
// #include <linux/types.h>
// #include <linux/idr.h>
// #include <linux/ktime.h>
// #include <linux/err.h>
// #include <linux/kobject.h>

uint8_t i = 0;
#define EXIT_SUCCESS 0
#define EXIT_FAILURE -1

#define RD_ONLY 0x01
#define WR_ONLY 0x10
#define RDWR 0x11
#define DEVICE_NAME "my_platform_device"
#define CLASS_NAME "my_platform_class"

#define MAX_DEVICES 5

void my_platform_device_release(struct device *dev);
int my_platform_device_probe(struct device *my_platform_device);
// int my_platform_device_remove(struct device *my_platform_device);
int my_platform_driver_probe(struct platform_device *my_platform_device);
int my_platform_driver_remove(struct platform_device *my_platform_device);

loff_t pcd_llseek(struct file *filp, loff_t offset, int whence);
ssize_t pcd_read(struct file *filp, char __user *buf, size_t count, loff_t *position);
ssize_t pcd_write(struct file *filp, const char __user *buf, size_t count, loff_t *position);
int pcd_release(struct inode *inode, struct file *filp);
int pcd_open(struct inode *inode, struct file *filp);

struct device_driver my_driver;
struct platform_driver my_platform_driver1;
struct platform_device *my_platform_device;

struct file_operations pcd_fops;
struct new_utsname my_kernel_version;
// struct pcdevice_private_data *my_device_private_data;
struct pcdriver_private_data my_driver_private_data;

struct pcdevice_platform_data
{
    int size;
    int perm;
    const char *serial_number;
};

struct pcdevice_private_data
{
    struct pcdevice_platform_data my_driver_platform_data;
    char *buffer;
    dev_t device_number;
    struct cdev *cdev;
};

struct pcdriver_private_data
{
    int total_devices;
    dev_t device_num_base;
    struct class *class;
    struct device *device;
    // struct platform_device *pdev;
    // struct file_operations *fops;
};
#endif /* _PLATFORM_H_ */