#include "my_platform.h"

struct platform_driver_private_data driver_private_data;

struct platform_driver my_platform_driver = {
    .probe = my_platform_driver_probe,
    .remove = my_platform_driver_remove,
    .driver = {
        .name = DEVICE_NAME,
        .owner = THIS_MODULE,
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

// for multiple drivers, you can use an array
// static struct platform_driver *my_platform_drivers[] = {
//     &my_platform_driver,
// };

#if 0
static int __init platform_driver_init(void)
{
    int ret = 0;

    /* 1. Register the platform driver */
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
#endif

static int __init platform_driver_init(void)
{
    int ret = 0;

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
    // platform_register_drivers(&my_platform_driver, MAX_DEVICES);

    pr_info("Platform Driver Registered [name] = [%s]\n", my_platform_driver.driver.name);

    return EXIT_SUCCESS;
}

static void __exit platform_driver_exit(void)
{
    platform_driver_unregister(&my_platform_driver);
    class_destroy(driver_private_data.driver_class);
    unregister_chrdev_region(driver_private_data.device_num_base, MAX_DEVICES);

    pr_info("Platform driver unregistered\n");
}

int my_platform_driver_probe(struct platform_device *probed_platform_device)
{
    int ret = 0;
    struct pcdev_platform_data *pdata; // device driver data
    struct platform_device_private_data *device_private_data;

    pr_info("Platform Device PROBED [name : id] = [ %s : %d]\n", probed_platform_device->name, probed_platform_device->id);

    /* 1. Get platform data from the probed device */
    // pdata = probed_platform_device->dev.platform_data;
    pdata = (struct pcdev_platform_data *)dev_get_platdata(&probed_platform_device->dev);
    if (!pdata)
    {
        pr_err("Platform data is not available for device %s\n", probed_platform_device->name);
        return -EINVAL;
    }
    pr_info("Platform Data: Serial Number: %s, Size: %zu, Permissions: %d\n", pdata->device_serial_number, pdata->size, pdata->permissions);

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

    device_private_data->buffer = devm_kmalloc(&probed_platform_device->dev, pdata->size, GFP_KERNEL);
    if (!device_private_data->buffer)
    {
        pr_err("Failed to allocate memory for device buffer\n");
        kfree(device_private_data);
        return -ENOMEM;
    }

    /* 3. Copy device platform data to device private data */
    memcpy(&device_private_data->pdata, pdata, sizeof(*pdata));
    // device_private_data->pdata.permissions = pdata->permissions;
    // device_private_data->pdata.device_serial_number = pdata->device_serial_number;
    // device_private_data->pdata.size = pdata->size;
    // pr_info("Device Private Data: Serial Number: %s, Size: %zu, Permissions: %d\n",
    //         device_private_data->pdata.device_serial_number,
    //         device_private_data->pdata.size,
    //         device_private_data->pdata.permissions);

    /* 4. Allocate buffer for device */
    device_private_data->buffer = kmalloc(device_private_data->pdata.size, GFP_KERNEL);
    if (!device_private_data->buffer)
    {
        pr_err("Failed to allocate memory for device buffer\n");
        kfree(device_private_data);
        return -ENOMEM;
    }

    /* 5. Get device number and driver data */
    device_private_data->device_number = driver_private_data.device_num_base + probed_platform_device->dev.devt; // devt is the device number

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

    /* 7. Create the device */
    driver_private_data.driver_device = device_create(driver_private_data.driver_class, NULL, device_private_data->device_number, NULL, "Device created for my_platform_device_%d", probed_platform_device->id);

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
    printk(KERN_INFO "Platform device number = %d\n", probed_platform_device->dev.devt);

    driver_private_data.total_devices++;
    pr_info("Total devices created: %d\n", driver_private_data.total_devices);

    /* this is used to passed the all the device data to another function Remove Function */
    /* store the device private data to specific platform device */
    // platform_set_drvdata(probed_platform_device, device_private_data);

    // probed_platform_device->dev.driver_data = device_private_data;
    dev_set_drvdata(&probed_platform_device->dev, device_private_data);

    return EXIT_SUCCESS;
}

int my_platform_driver_remove(struct platform_device *my_platform_device)
{
    struct platform_device_private_data *device_private_data;

    device_private_data = dev_get_drvdata(&my_platform_device->dev);
    if (!device_private_data)
    {
        pr_err("Failed to get device private data\n");
        return -EINVAL;
    }

    device_destroy(driver_private_data.driver_class, device_private_data->device_number);
    cdev_del(&device_private_data->cdev);
    kfree(device_private_data->buffer);
    kfree(device_private_data);

    driver_private_data.total_devices--;

    pr_info("Platform Device REMOVED [name : id] = [ %s : %d]\n", my_platform_device->name, my_platform_device->id);

    pr_info("Total available devices: %d\n", driver_private_data.total_devices);

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