#include "my_platform.h"

struct platform_device *my_platform_device;
struct pcdev_platform_data pdata =
    {
        .device_serial_number = "my_platform_expt_device",
        .size = 1024,
        .permissions = RDWR,
};

static int __init platform_device_init(void)
{
    int ret = 0;
    my_platform_device = platform_device_alloc(DEVICE_NAME, DEVICE_ID);
    if (!my_platform_device)
    {
        pr_err("Failed to allocate platform device\n");
        return -ENOMEM;
    }

    my_platform_device->dev.platform_data = &pdata; // platform data is the data which we are sending from the device

    ret = platform_device_add(my_platform_device);
    if (ret < 0)
    {
        pr_err("Failed to add platform device\n");
        platform_device_put(my_platform_device);
        return ret;
    }

    printk(KERN_INFO "Platform device is initialized [name : id] = [%s : %d] \n", my_platform_device->name, my_platform_device->id);

    return ret;
}

static void __exit platform_device_exit(void)
{

    if (my_platform_device)
    {
        my_platform_device->dev.platform_data = NULL;
        platform_device_unregister(my_platform_device);
        pr_info("Platform device unregistered successfully\n");
    }
    else
    {
        pr_info("Platform device already unregistered or not initialized\n");
    }
}

module_init(platform_device_init);
module_exit(platform_device_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Platform Device Setup");
MODULE_AUTHOR("HDA");