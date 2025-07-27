#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/errno.h>

uint8_t i = 0;
#define EXIT_SUCCESS 0
#define EXIT_FAILURE -1

#define DEVICE_NAME "pseudo_char_device"
#define CLASS_NAME "pseudo_char_class"
#define MAX_DEVICE_FILE 4 // Maximum number of devices supported

#define DEV_MEM_SIZE_PCD_1 512 // Size of the device 1 memory buffer
#define DEV_MEM_SIZE_PCD_2 512 // Size of the device 2 memory buffer
#define DEV_MEM_SIZE_PCD_3 512 // Size of the device 3 memory buffer
#define DEV_MEM_SIZE_PCD_4 512 // Size of the device 4 memory buffer

static int __init pcd_driver_init(void);
static void __exit pcd_driver_exit(void);

loff_t pcd_llseek(struct file *filp, loff_t offset, int whence);
ssize_t pcd_read(struct file *filp, char __user *buf, size_t count, loff_t *position);
ssize_t pcd_write(struct file *filp, const char __user *buf, size_t count, loff_t *position);
int pcd_release(struct inode *inode, struct file *filp);
int pcd_open(struct inode *inode, struct file *filp);

char device_buffer_pcd_1[DEV_MEM_SIZE_PCD_1];
char device_buffer_pcd_2[DEV_MEM_SIZE_PCD_2];
char device_buffer_pcd_3[DEV_MEM_SIZE_PCD_3];
char device_buffer_pcd_4[DEV_MEM_SIZE_PCD_4];

// Define the private data structure for the pseudo character device driver
struct pcdev_private_data
{
    char *buffer;              // Pointer to the device buffer
    size_t size;               // Size of the device buffer
    int minor;                 // Minor number of the device
    int permissions;           // Permissions for the device
    const char *serial_number; // Serial number of the device

    struct cdev cdev;
};

struct pcdrv_private_data
{
    int total_devices;         // Total number of devices
    dev_t device_number;       // Device number for the device
    struct class *class_pcd;   // Pointer to the device class
    struct device *device_pcd; // Pointer to the device
    struct pcdev_private_data
        pcdev_data[MAX_DEVICE_FILE]; // Pointer to the array of device private data
};

struct pcdrv_private_data pcdrv_data = {
    .total_devices = MAX_DEVICE_FILE,
    .class_pcd = NULL,  // Will be initialized later
    .device_pcd = NULL, // Will be initialized later
    .pcdev_data = {
        [0] = {
            .buffer = device_buffer_pcd_1,
            .size = DEV_MEM_SIZE_PCD_1,
            .minor = 0,
            .permissions = 0666,
            .serial_number = "pseudo_char_device_1",
            .cdev = {
                .owner = THIS_MODULE, // current module identification
                .ops = NULL,          // Will be set later
                .dev = 0,
                .count = 1,
            },
        },
        [1] = {
            .buffer = device_buffer_pcd_2,
            .size = DEV_MEM_SIZE_PCD_2,
            .minor = 1,
            .permissions = 0666,
            .serial_number = "pseudo_char_device_2",
            .cdev = {
                .owner = THIS_MODULE, // current module identification
                .ops = NULL,          // Will be set later
                .dev = 0,
                .count = 1,
            },
        },
        [2] = {
            .buffer = device_buffer_pcd_3,
            .size = DEV_MEM_SIZE_PCD_3,
            .minor = 2,
            .permissions = 0666,
            .serial_number = "pseudo_char_device_3",
            .cdev = {
                .owner = THIS_MODULE, // current module identification
                .ops = NULL,          // Will be set later
                .dev = 0,
                .count = 1,
            },
        },
        [3] = {
            .buffer = device_buffer_pcd_4,
            .size = DEV_MEM_SIZE_PCD_4,
            .minor = 3,
            .permissions = 0666,
            .serial_number = "pseudo_char_device_4",
            .cdev = {
                .owner = THIS_MODULE, // current module identification
                .ops = NULL,          // Will be set later
                .dev = 0,
                .count = 1,
            },
        },
    },
};

struct file_operations pcd_fops = {
    .owner = THIS_MODULE,
    .read = pcd_read,       // Define read function if needed
    .write = pcd_write,     // Define write function if needed
    .open = pcd_open,       // Define open function if needed
    .release = pcd_release, // Define release function if needed
    .llseek = pcd_llseek,   // Define llseek function if needed
};

static int __init pcd_driver_init(void)
{
    printk(KERN_INFO "\n\n****Pseudo Character Device Driver Initialized***\n");

    /* 1. Allocate a device number in kernel space region*/
    alloc_chrdev_region(&pcdrv_data.device_number, 0, MAX_DEVICE_FILE, DEVICE_NAME);

    if (pcdrv_data.device_number < 0)
    {
        printk(KERN_ALERT "Failed to allocate device number\n");
        return pcdrv_data.device_number;
    }

    /* 2. Create Device class only once under /sys/class*/
    pcdrv_data.class_pcd = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(pcdrv_data.class_pcd)) // convert void pointer to struct class pointer
    {
        printk(KERN_ALERT "Failed to create device class\n");
        unregister_chrdev_region(pcdrv_data.device_number, MAX_DEVICE_FILE); // Unregister the device number
        return PTR_ERR(pcdrv_data.class_pcd);                                // convert void pointer to int pointer
    }
    printk(KERN_INFO "Device class created successfully\n");

    pr_info("%s : Device number allocated for files \n", __func__);
    /* 3. Register the character device with VFS */
    /*  for multiple devices, we need to create cdev variable 10 devices, we need to create 10 cdev variables         */

    for (i = 0; i < pcdrv_data.total_devices; i++)
    {
        pr_info("\n[device_file = %d] Major:Minor = %d, %d\n", (i + 1), MAJOR(pcdrv_data.device_number + i), MINOR(pcdrv_data.device_number + i));

        cdev_init(&pcdrv_data.pcdev_data[i].cdev, &pcd_fops);
        pcdrv_data.pcdev_data[i].cdev.owner = THIS_MODULE;                             // Set the owner of the cdev
        pcdrv_data.pcdev_data[i].cdev.ops = &pcd_fops;                                 // Set the file operations for the cdev
        pcdrv_data.pcdev_data[i].cdev.dev = MKDEV(MAJOR(pcdrv_data.device_number), i); // Set the device number for the cdev

        /* 4. Add the char device to the system for 1 file */
        if (cdev_add(&pcdrv_data.pcdev_data[i].cdev, pcdrv_data.device_number + i, 1) < 0)
        {
            printk(KERN_ALERT "Failed to add cdev\n");
            unregister_chrdev_region(pcdrv_data.device_number + i, MAX_DEVICE_FILE);
            return EXIT_FAILURE;
        }

        /* 5. Create Device under /dev - Populate the sysfs with device information*/
        pcdrv_data.device_pcd = device_create(pcdrv_data.class_pcd, NULL, pcdrv_data.device_number + i, NULL, pcdrv_data.pcdev_data[i].serial_number);

        if (IS_ERR(pcdrv_data.device_pcd))
        {
            printk(KERN_ALERT "Failed to create device\n");
            cdev_del(&pcdrv_data.pcdev_data[i].cdev);                            // Remove the cdev
            class_destroy(pcdrv_data.class_pcd);                                 // Destroy the class
            unregister_chrdev_region(pcdrv_data.device_number, MAX_DEVICE_FILE); // Unregister the device number
            return PTR_ERR(pcdrv_data.device_pcd);
        }

        printk(KERN_INFO "Device created successfully: %s\n", pcdrv_data.pcdev_data[i].serial_number);
    }

    printk(KERN_INFO "Pseudo Character Device Driver Initialization Complete\n");

    return EXIT_SUCCESS;
}

static void __exit pcd_driver_exit(void)
{
    printk(KERN_INFO "Pseudo Character Device Driver Exited\n");

    for (i = 0; i < pcdrv_data.total_devices; i++)
    {

        /* 1. Remove the device from sysfs */
        device_destroy(pcdrv_data.class_pcd, pcdrv_data.device_number);

        /* 2. Unregister the device class */
        class_destroy(pcdrv_data.class_pcd);

        /* 3. Remove the cdev from the system */
        cdev_del(&pcdrv_data.pcdev_data[i].cdev);

        // cdev_deinit(&cdev); // Deinitialize the cdev
        //  Free the device number
        printk(KERN_INFO "Freeing device number: %d:%d\n", MAJOR(pcdrv_data.device_number + i), MINOR(pcdrv_data.device_number + i));
        // Note: The device number is freed automatically when the cdev is removed
        // If you want to explicitly free it, you can use unregister_chrdev_region
        // but it's not necessary here as we are already removing the cdev.

        /* 5. Unregister the character device region */
        unregister_chrdev_region(pcdrv_data.device_number, MAX_DEVICE_FILE);
    }

    printk(KERN_INFO "Pseudo Character Device Driver Cleanup Complete\n");
}

#if 0

loff_t pcd_llseek(struct file *filp, loff_t offset, int whence)
{
    pr_info("pcd_llseek called with offset: %lld, whence: %d\n", offset, whence);

    switch (whence)
    {
    case SEEK_SET:
    {
        if (offset < 0 || offset > DEV_MEM_SIZE)
        {
            pr_err("Invalid offset for SEEK_SET\n");
            return -EINVAL; // Invalid argument
        }
        /* Set the file position to the specified offset */
        filp->f_pos = offset;
        break;
    }
    case SEEK_CUR:
    {
        if (filp->f_pos + offset < 0 || filp->f_pos + offset > DEV_MEM_SIZE)
        {
            pr_err("Invalid offset for SEEK_CUR\n");
            return -EINVAL; // Invalid argument
        }
        filp->f_pos += offset;
        break;
    }
    case SEEK_END:
    {
        if (DEV_MEM_SIZE + offset < 0 || DEV_MEM_SIZE + offset > DEV_MEM_SIZE)
        {
            pr_err("Invalid offset for SEEK_END\n");
            return -EINVAL; // Invalid argument
        }
        filp->f_pos = DEV_MEM_SIZE + offset;
        break;
    }
    default:
    {
        pr_err("Invalid whence value\n");
        return -EINVAL; // Invalid argument
    }
    }

    pr_info("New file position: %lld\n", filp->f_pos);
    return filp->f_pos; // Return the new file position
}

ssize_t pcd_read(struct file *filp, char __user *buf, size_t count, loff_t *position)
{

    pr_info("Before pcd_read: Read %zu bytes from file position %lld\n", count, *position);

    if (*position >= DEV_MEM_SIZE)
    {
        pr_info("End of file reached, no more data to read\n");
        return 0; // End of file
    }

    /* Check for excess beyond buffer size - 512 Bytes */
    if (*position + count > DEV_MEM_SIZE)
    {
        pr_info("Adjusting count to prevent overflow\n");
        count = DEV_MEM_SIZE - *position; // Adjust count to prevent overflow
    }

    /* Copy data from device buffer to user space */
    if (copy_to_user(buf, device_buffer + *position, count) != 0)
    {
        pr_err("Failed to copy data to user space\n");
        return -EFAULT; // Return error if copy fails
    }

    /* Update the file position */
    *position += count;

    pr_info("After pcd_read: Read %zu bytes from file position %lld\n", count, *position);

    return count; // Return the number of bytes read
}

ssize_t pcd_write(struct file *filp, const char __user *buf, size_t count, loff_t *position)
{
    pr_info("Before pcd_write: Wrote %zu bytes to file position %lld\n", count, *position);

    if (count == 0)
    {
        return -ENOMEM; // Nothing to write
    }

    /* Check for excess beyond buffer size - 512 Bytes */
    if (*position + count > DEV_MEM_SIZE)
    {
        count = DEV_MEM_SIZE - *position; // Adjust count to prevent overflow
    }

    /* Copy data from user space to device buffer */
    if (copy_from_user(device_buffer + *position, buf, count) != 0)
    {
        pr_err("Failed to copy data from user space\n");
        return -EFAULT; // Return error if copy fails
    }

    /* Update the file position */
    *position += count;

    pr_info("After pcd_write: Wrote %zu bytes to file position %lld\n", count, *position);

    return count; // Return the number of bytes successfuly written
}
#endif

int pcd_release(struct inode *inode, struct file *filp)
{
    if (filp->f_mode & FMODE_READ)
    {
        pr_info("\n***pcd_release for read is successful***\n\n");
    }
    else
    {
        pr_info("\n***pcd_release for write is successful***\n\n");
    }
    return EXIT_SUCCESS;
}

int pcd_open(struct inode *inode, struct file *filp)
{
    int ret = 0;
    int minor_number;

    minor_number = MINOR(inode->i_rdev); // Get the minor number from the inode
    pr_info("pcd_open called for minor number: %d\n", minor_number);
    
    if (minor_number >= pcdrv_data.total_devices)
    {
        pr_err("Invalid minor number: %d\n", minor_number);
        return -ENODEV; // No such device
    }
    filp->private_data = &pcdrv_data.pcdev_data[minor_number]; // Store the private data in the file structure
    pr_info("pcd_open: Device %s opened successfully\n", pcdrv_data.pcdev_data[minor_number].serial_number);

        if (filp->f_mode & FMODE_READ)
    {
        pr_info("\n\n***pcd_open for read is successful***\n");
    }
    else
    {
        pr_info("\n\n***pcd_open for write is successful***\n");
    }
    return EXIT_SUCCESS;
}

module_init(pcd_driver_init);
module_exit(pcd_driver_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("HDA");
MODULE_DESCRIPTION("A simple pseudo character device driver");
MODULE_VERSION("0.1");
