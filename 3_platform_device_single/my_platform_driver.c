#include "my_platform.h"

struct platform_driver my_platform_driver = {
    .probe = my_platform_driver_probe,
    .remove = my_platform_driver_remove,
    .driver = {
        .name = DEVICE_NAME,
        .owner = THIS_MODULE,
    },
};

// for multiple drivers, you can use an array
// static struct platform_driver *my_platform_drivers[] = {
//     &my_platform_driver,
// };

static int __init platform_driver_init(void)
{
    int ret = 0;

    /* 3. Register the platform driver */
    // ret = platform_register_drivers(&my_platform_driver, 1); // This is used for multiple drivers, but we are using a single driver here.
    ret = platform_driver_register(&my_platform_driver);
    if (ret < 0)
    {
        pr_err("Failed to register platform driver: %d\n", ret);
    }

    // class_destroy(my_driver_private_data.class);

    pr_info("Platform Driver Initialized [name] = [%s]\n", my_platform_driver.driver.name);
    // kernel_version();
    return EXIT_SUCCESS;
}

static void __exit platform_driver_exit(void)
{
    platform_driver_unregister(&my_platform_driver);

    // class_destroy(my_driver_private_data.class);
    // my_driver_private_data.class = NULL;
    // unregister_chrdev_region(my_driver_private_data.device_num_base, MAX_DEVICES);

    pr_info("Platform driver unregistered\n");
}

int my_platform_driver_probe(struct platform_device *my_platform_device)
{
    pr_info("Platform Device PROBED [name : id] = [ %s : %d]\n", my_platform_device->name, my_platform_device->id);
    return EXIT_SUCCESS;
}

int my_platform_driver_remove(struct platform_device *my_platform_device)
{
    pr_info("Platform Device REMOVED [name : id] = [ %s : %d]\n", my_platform_device->name, my_platform_device->id);
    return EXIT_SUCCESS;
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

module_init(platform_driver_init);
module_exit(platform_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("HDA");
MODULE_DESCRIPTION("A simple platform device driver");
MODULE_VERSION("0.1");
