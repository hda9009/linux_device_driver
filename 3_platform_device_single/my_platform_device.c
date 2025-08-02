#include "my_platform.h"

// create a platform device
struct pcdevice_platform_data my_platform_data = {
    .size = 1024,
    .perm = RDWR,
    .serial_number = "my_platform_device_testing",
};

void my_platform_device_release(struct device *dev)
{
    printk(KERN_INFO "Platform device released\n");
}

struct platform_device my_platform_device = {
    .name = DEVICE_NAME,
    .id = 0,
    // .dev.release = my_platform_device_release,
    .dev = {
        .platform_data = &my_platform_data,
        // .release = platform_device_release,
    },
};

// struct pcdevice_private_data *my_device_private_data = {
//     .my_driver_platform_data = {
//         .size = 1024,
//         .perm = RDWR,
//         .serial_number = "my_driver_private_data_testing",
//     },
//     .buffer = NULL,
//     .device_number = 0,
//     .cdev = NULL,
// };

static int __init platform_device_init(void)
{
    int ret;
    pr_info("\n\nInitializing platform device...\n");

    ret = platform_device_register(&my_platform_device);
    if (ret < 0)
    {
        pr_err("Failed to register platform device: %d\n", ret);
        return -1;
    }
    printk(KERN_INFO "Platform device registered with name: %s\n", my_platform_device.name);
    printk(KERN_INFO "Platform device ID: %d\n", my_platform_device.id);
    return 0;
}

static void __exit platform_device_exit(void)
{
    // platform_device_del(&my_platform_device);
    printk(KERN_INFO "Platform device exited\n");
    platform_device_unregister(&my_platform_device);
}

EXPORT_SYMBOL_GPL(platform_device_init);
module_init(platform_device_init);
module_exit(platform_device_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Platform Device Setup");
MODULE_AUTHOR("HDA");