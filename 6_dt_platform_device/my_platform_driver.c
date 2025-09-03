#include "my_platform.h"

struct platform_driver_private_data driver_private_data;
// struct device_config_data my_device_config_data[MAX_DEVICES];
struct platform_device_id my_platform_device_ids[MAX_DEVICES];

// struct device_config_data my_device_config_data[MAX_DEVICES] = {
//     {.data = 100, .buffer = "config_data_1"},
//     {.data = 200, .buffer = "config_data_2"},
//     {.data = 300, .buffer = "config_data_3"},
// };

struct platform_device_id my_platform_device_ids[MAX_DEVICES] = {
    [DEVICE_ID_1] = {.name = "my_platform_device_1"},
    [DEVICE_ID_2] = {.name = "my_platform_device_2"},
    [DEVICE_ID_3] = {.name = "my_platform_device_3"},
};

#if 0
platform_device_dt_1 {
                        compatible = "my_platform_device_dt_compatible_1";
                        org,size = <512>;
                        org,device_serial_number = "my_platform_device_dt_1";
                        org,permissions = <0x11>;
    };
#endif

struct of_device_id org_platform_data[] =
    {
        {
            .compatible = "my_platform_device_dt_compatible_1",
            .data = (void *)"dt_device_1",
        },
        {
            .compatible = "my_platform_device_dt_compatible_2",
            .data = (void *)"dt_device_2",
        },
        {
            .compatible = "my_platform_device_dt_compatible_3",
            .data = (void *)"dt_device_3",
        },
};

struct platform_driver my_platform_driver = {
    .probe = my_platform_driver_probe,
    .remove = my_platform_driver_remove,
    .id_table = my_platform_device_ids,
    .driver = {
        .name = DRV_DEVICE_NAME,
        .owner = THIS_MODULE,
        .of_match_table = org_platform_data,
    },
};

struct file_operations pcd_fops = {
    .owner = THIS_MODULE,
    .llseek = pcd_llseek,
    .read = pcd_read,
    .write = pcd_write,
    .open = pcd_open,
    .release = pcd_release,
};

static int __init platform_driver_init(void)
{
    int ret = 0;

    pr_info("\n\n**** Driver Initialization***** \n");

    /* 1. Allocate character device region & get the device base number */
    ret = alloc_chrdev_region(&driver_private_data.device_num_base, 0, MAX_DEVICES, DRV_DEVICE_NAME); // change device name to pcdevs
    if (driver_private_data.device_num_base < 0)
    {
        pr_err("Failed to allocate character device region: %d\n", ret);
        return ret;
    }

    /* 2. Create device class of the driver*/
    driver_private_data.driver_class = class_create(THIS_MODULE, CLASS_NAME);

    if (IS_ERR(driver_private_data.driver_class))
    {
        pr_err("Failed to create class: %ld\n", PTR_ERR(driver_private_data.driver_class));
        unregister_chrdev_region(driver_private_data.device_num_base, MAX_DEVICES);
        return PTR_ERR(driver_private_data.driver_class);
    }

    /* 3. device create will execute when probe function is called */

    /* 4. Register the platform driver*/
    platform_driver_register(&my_platform_driver);

    pr_info("Platform Driver Registered [name] = [%s]\n", my_platform_driver.driver.name);
    pr_info("*** Driver Initialization Completed***\n\n");

    return EXIT_SUCCESS;
}

static void __exit platform_driver_exit(void)
{
    pr_info("Platform driver unregistered\n");
    platform_driver_unregister(&my_platform_driver);
    class_destroy(driver_private_data.driver_class);
    unregister_chrdev_region(driver_private_data.device_num_base, MAX_DEVICES);
}

struct pcdev_platform_data *platform_get_data_from_device_tree(struct device *probed_platform_device_data)
{
    struct device_node *dev_node = probed_platform_device_data->of_node;
    struct pcdev_platform_data *pdata;

    if (dev_node == NULL)
    {
        pr_err("Device is not added through DT, node is NULL\n");
        return NULL;
    }

    pdata = devm_kmalloc(probed_platform_device_data, sizeof(*pdata), GFP_KERNEL);
    if (!pdata)
    {
        dev_info(probed_platform_device_data, "Failed to allocate memory for platform data\n");
        return ERR_PTR(-ENOMEM);
    }
    if (of_property_read_string(dev_node, "org,device_serial_number", &pdata->device_serial_number))
    {
        dev_info(probed_platform_device_data, "Failed to read device serial number\n");
        return ERR_PTR(-EINVAL);
    }
    if (of_property_read_u32(dev_node, "org,size", &pdata->size))
    {
        dev_info(probed_platform_device_data, "Failed to read device size\n");
        return ERR_PTR(-EINVAL);
    }

    if (of_property_read_u32(dev_node, "org,permissions", &pdata->permissions))
    {
        dev_info(probed_platform_device_data, "Failed to read device permissions\n");
        return ERR_PTR(-EINVAL);
    }

    return pdata;
}

int my_platform_driver_probe(struct platform_device *probed_platform_device)
{
    int ret = 0;
    struct pcdev_platform_data *pdata; // device driver data
    struct platform_device_private_data *device_private_data;
    int driver_data = 0;
    // struct of_device_id *match;

    pr_info("Platform Device PROBED [name : id] = [ %s : %d]\n", probed_platform_device->name, probed_platform_device->id);

    if (probed_platform_device->dev.of_node != NULL)
    {
        pr_info("Device is added through Device Tree\n");
        pdata = platform_get_data_from_device_tree(&probed_platform_device->dev);
        if (pdata == NULL)
        {
            pr_err("Failed to get platform data from device tree\n");
            return -EINVAL;
        }

        /* extracting org_platform_data driver data*/
        // match = of_match_device(of_match_ptr(org_platform_data), &probed_platform_device->dev);
        // match = of_match_device(probed_platform_device->dev.driver->of_match_table, &probed_platform_device->dev);
        // if (!match)
        // {
        //     pr_err("Failed to match device with device tree\n");
        //     return -EINVAL;
        // }
        // driver_data = (int)(uintptr_t)match->data;
        driver_data = (int)of_device_get_match_data(&probed_platform_device->dev);
    }
    else
    {
        pr_info("Device is NOT added through Device Tree\n");
        pdata = (struct pcdev_platform_data *)dev_get_platdata(&probed_platform_device->dev);
        if (pdata == NULL)
        {
            pr_err("Platform data is not available for device %s\n", probed_platform_device->name);
            return -EINVAL;
        }

        driver_data = probed_platform_device->id_entry->driver_data;
    }

    pr_info("Platform Data: Serial Number: %s\n", pdata->device_serial_number);

    // pr_info("Platform Data: Serial Number: %s, Size: %zu, Permissions: %d\n", pdata->device_serial_number, pdata->size, pdata->permissions);

    /* 2. Allocate memory for device private data */
    device_private_data = kmalloc(sizeof(*device_private_data), GFP_KERNEL);
    if (!device_private_data)
    {
        pr_err("Failed to allocate memory for device private data\n");
        return -ENOMEM;
    }
    device_private_data->buffer = kmalloc(pdata->size, GFP_KERNEL);
    if (!device_private_data->buffer)
    {
        pr_err("Failed to allocate memory for device buffer\n");
        kfree(device_private_data);
        return -ENOMEM;
    }

    /* 3. Copy device platform data to device private data */
    memcpy(&device_private_data->pdata, pdata, sizeof(*pdata));

    /* 5. Get device number and driver data */
    device_private_data->device_number = driver_private_data.device_num_base + driver_private_data.total_devices; // devt is the device number

    pr_info("Device Number = %d \n", device_private_data->device_number);

    /* 6. CDEV Init & CDEV add */
    cdev_init(&device_private_data->cdev, &pcd_fops);
    device_private_data->cdev.owner = THIS_MODULE;
    device_private_data->cdev.ops = &pcd_fops;

    ret = cdev_add(&device_private_data->cdev, device_private_data->device_number, 1);
    if (ret)
    {
        pr_err("Failed to add cdev: %d\n", ret);
        kfree(device_private_data->buffer);
        kfree(device_private_data);
        kfree(pdata);
        return ret;
    }

    driver_private_data.total_devices++;
    pr_info("Total devices created: %d\n", driver_private_data.total_devices);

    dev_set_drvdata(&probed_platform_device->dev, device_private_data);

    /* 7. Create the device */
    driver_private_data.driver_device = device_create(driver_private_data.driver_class, NULL, device_private_data->device_number, NULL, "Device created for my_platform_device_%d", driver_private_data.total_devices);

    if (IS_ERR(driver_private_data.driver_device))
    {
        pr_err("Failed to create device: %ld\n", PTR_ERR(driver_private_data.driver_device));
        cdev_del(&device_private_data->cdev);
        kfree(device_private_data->buffer);
        kfree(device_private_data);
        class_destroy(driver_private_data.driver_class);
        unregister_chrdev_region(driver_private_data.device_num_base, MAX_DEVICES);
        return PTR_ERR(driver_private_data.driver_device);
    }

    pr_info("Device created successfully [name: %s, id: %d, device number: %d]\n", probed_platform_device->name, probed_platform_device->id, device_private_data->device_number);

    // pr_info("Config data = %d, buffer = %s\n", my_device_config_data[probed_platform_device->id].data, my_device_config_data[probed_platform_device->id].buffer);
    // pr_info("Config data = %d, buffer = %s\n", my_device_config_data[driver_data].data, my_device_config_data[driver_data].buffer);

    // printk(KERN_INFO "Platform device number = %d\n", probed_platform_device->dev.devt);

    pdata = NULL;
    device_private_data = NULL;
    kfree(pdata);
    kfree(device_private_data);

    return EXIT_SUCCESS;
}

int my_platform_driver_remove(struct platform_device *my_platform_device)
{
    pr_info("Platform Device REMOVED [name : id] = [ %s : %d]\n", my_platform_device->name, my_platform_device->id);
#if 0
    struct platform_device_private_data *device_private_data;

    device_private_data = dev_get_drvdata(&my_platform_device->dev);
    if (!device_private_data)
    {
        pr_err("Failed to get device private data\n");
        return -EINVAL;
    }

    device_destroy(driver_private_data.driver_class, device_private_data->device_number);
    cdev_del(&device_private_data->cdev);

    driver_private_data.total_devices--;

    pr_info("Platform Device REMOVED [name : id] = [ %s : %d]\n", my_platform_device->name, my_platform_device->id);
    pr_info("Total available devices: %d\n", driver_private_data.total_devices);

#endif

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