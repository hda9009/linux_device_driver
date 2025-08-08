#include "my_platform.h"

struct platform_driver my_platform_driver1 = {
    .probe = my_platform_driver_probe,
    .remove = my_platform_driver_remove,
    .driver = {
        .name = DEVICE_NAME,
        .owner = THIS_MODULE,
    },
};

// static struct platform_driver *my_platform_drivers[] = {
//     &my_platform_driver1,
// };

// struct pcdriver_private_data my_driver_private_data = {
//     .total_devices = MAX_DEVICES,
//     .device_num_base = 0,
//     // .class = NULL,
//     // .device = NULL,
//     // .pdev = NULL,
//     // .fops = NULL,
// };

static int __init platform_driver_init(void)
{
    int ret;
    // int number_of_drivers = ARRAY_SIZE(my_platform_drivers);

    // /* 1. Allocate character device region */
    // ret = alloc_chrdev_region(&my_driver_private_data.device_num_base, 0, MAX_DEVICES, DRIVER_NAME);

    // if (ret < 0)
    // {
    //     pr_err("Failed to allocate character device region: %d\n", ret);
    //     return EXIT_FAILURE;
    // }

    // /* 2. Create Class of the driver */
    // my_driver_private_ data.class = class_create(THIS_MODULE, CLASS_NAME);
    // if (IS_ERR(my_driver_private_data.class))
    // {
    //     pr_err("Failed to create class: %ld\n", PTR_ERR(my_driver_private_data.class));
    //     unregister_chrdev_region(my_driver_private_data.device_num_base, MAX_DEVICES);
    //     return EXIT_FAILURE;
    // }

    // struct platform_driver *my_platform_driver = platform_driver_alloc();
    // if (!my_platform_driver)
    // {
    //     pr_err("Failed to allocate platform driver\n");
    // }

    // my_platform_drivers[0]->driver.owner = THIS_MODULE;
    // my_platform_drivers[0]->driver.name = DEVICE_NAME;
    // my_platform_drivers[0]->driver.probe = my_platform_device_probe;
    // my_platform_drivers[0]->probe = my_platform_driver_probe;
    // my_platform_drivers[0]->remove = my_platform_driver_remove;
    // ret = platform_register_drivers(&my_platform_driver1, 1);
    // class_destroy(my_driver_private_data.class);

    ret = platform_driver_register(&my_platform_driver1);
    if (ret < 0)
    {
        pr_err("Failed to register platform driver: %d\n", ret);
    }

    pr_info("Platform driver initialized with name: %s\n", my_platform_driver1.driver.name);
    // kernel_version();
    return EXIT_SUCCESS;
}

static void __exit platform_driver_exit(void)
{
    platform_driver_unregister(&my_platform_driver1);

    // class_destroy(my_driver_private_data.class);
    // my_driver_private_data.class = NULL;
    // unregister_chrdev_region(my_driver_private_data.device_num_base, MAX_DEVICES);

    pr_info("Platform driver unregistered\n");
}

loff_t pcd_llseek(struct file *filp, loff_t offset, int whence)
{
    pr_info("\n***pcd_llseek is successful***\n\n");
    return 0;
}

ssize_t pcd_read(struct file *filp, char __user *buf, size_t count, loff_t *position)
{
    pr_info("\n***pcd_read is successful***\n\n");
    return 0;
}

ssize_t pcd_write(struct file *filp, const char __user *buf, size_t count, loff_t *position)
{
    pr_info("\n***pcd_write is successful***\n\n");
    return 0;
}

int pcd_release(struct inode *inode, struct file *filp)
{
    pr_info("\n***pcd_release is successful***\n\n");
    return EXIT_SUCCESS;
}

int pcd_open(struct inode *inode, struct file *filp)
{
    pr_info("\n***pcd_open is successful***\n\n");
    return 0;
}

int my_platform_driver_probe(struct platform_device *my_platform_device)
{
    pr_info("\nPlatform driver probed for device: %s\n", my_platform_device->name);
    pr_info("Platform device ID: %d\n\n", my_platform_device->id);

    return EXIT_SUCCESS;
}

int my_platform_driver_remove(struct platform_device *my_platform_device)
{
    pr_info("Platform driver removed for device: %s\n", my_platform_device->name);
    pr_info("Platform device ID: %d\n", my_platform_device->id);
    return EXIT_SUCCESS;
}

int my_platform_device_probe(struct device *my_platform_device)
{
    pr_info("Platform device probed: %s\n", my_platform_device->init_name);
    pr_info("Platform device ID: %d\n", my_platform_device->id);

    // Initialize platform device specific data here
    // For example, allocate memory or set up resources

    return EXIT_SUCCESS;
}

module_init(platform_driver_init);
module_exit(platform_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("HDA");
MODULE_DESCRIPTION("A simple platform device driver");
MODULE_VERSION("0.1");
