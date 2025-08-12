#include "my_platform.h"

struct platform_device *my_platform_device[MAX_DEVICES];

struct pcdev_platform_data pdata[] =
    {
        {
            .device_serial_number = "my_platform_expt_device_1",
            .size = 1024,
            .permissions = RDWR,
        },
        {
            .device_serial_number = "my_platform_expt_device_2",
            .size = 2048,
            .permissions = RDWR,
        },
        {
            .device_serial_number = "my_platform_expt_device_3",
            .size = 4096,
            .permissions = RDWR,
        },
};

struct platform_device_details device_details[] =
    {
        {.name = "my_platform_device_1",
         .id = DEVICE_ID_1},
        {.name = "my_platform_device_2",
         .id = DEVICE_ID_2},
        {.name = "my_platform_device_3",
         .id = DEVICE_ID_3},
};

static int __init platform_device_init(void)
{
    int ret = 0;
    int device_count = 0;

    pr_info("\n\n***** Device Initialization *****\n");
    for (device_count = 0; device_count < MAX_DEVICES; device_count++)
    {
        my_platform_device[device_count] = platform_device_alloc(device_details[device_count].name, device_details[device_count].id);

        if (!my_platform_device[device_count])
        {
            pr_err("Failed to allocate platform device\n");
            return -ENOMEM;
        }
        my_platform_device[device_count]->dev.platform_data = &pdata[device_count];

        ret = platform_device_add(my_platform_device[device_count]);
        if (ret < 0)
        {
            pr_err("Failed to add platform device\n");
            platform_device_put(my_platform_device[device_count]);
            return ret;
        }

        printk(KERN_INFO "Platform device is initialized [name : id] = [%s : %d] \n", my_platform_device[device_count]->name, my_platform_device[device_count]->id);
    }
    pr_info("\n*****Device Initialization Completed*****\n");
    return ret;
}

static void __exit platform_device_exit(void)
{
    int device_count = 0;

    for (device_count = 0; device_count < MAX_DEVICES; device_count++)
    {
        if (my_platform_device[device_count])
        {
            my_platform_device[device_count]->dev.platform_data = NULL;
            platform_device_unregister(my_platform_device[device_count]);
            pr_info("Platform device unregistered successfully\n");
        }
        else
        {
            pr_info("Platform device already unregistered or not initialized\n");
        }
    }
}

module_init(platform_device_init);
module_exit(platform_device_exit);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Platform Device Setup");
MODULE_AUTHOR("HDA");