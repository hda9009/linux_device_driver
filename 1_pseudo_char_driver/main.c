#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/errno.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE -1

#define DEVICE_NAME "pseudo_char_device"
#define CLASS_NAME "pseudo_char_class"
#define DEV_MEM_SIZE 512 // Size of the device memory buffer

static int __init pcd_driver_init(void);
static void __exit pcd_driver_exit(void);

loff_t pcd_llseek(struct file *filp, loff_t offset, int whence);
ssize_t pcd_read(struct file *filp, char __user *buf, size_t count, loff_t *position);
ssize_t pcd_write(struct file *filp, const char __user *buf, size_t count, loff_t *position);
int pcd_release(struct inode *inode, struct file *filp);
int pcd_open(struct inode *inode, struct file *filp);

char device_buffer[DEV_MEM_SIZE];
dev_t device_number;
struct class *class_pcd;   // Pointer to the device class
struct device *device_pcd; // Pointer to the device

struct cdev pcd_cdev = {
    .owner = THIS_MODULE, // current module identification
    .ops = NULL,          // Will be set later
    .dev = 0,
    .count = 1,
    .kobj = {
        .name = DEVICE_NAME,
        .state_initialized = 1,
        .state_in_sysfs = 0,
        .state_add_uevent_sent = 0,
        .state_remove_uevent_sent = 0,
        .uevent_suppress = 0,
    },
    //.file_operations = NULL,
    .list = {
        .next = NULL,
        .prev = NULL,
    },
};

#if 0
struct file *pcd_file = {
    .f_u = {
        .fu_llist = {
            .next = NULL,
            .prev = NULL,
        },
    },
    .f_path = {
        .dentry = NULL,
        .mnt = NULL,
    },
    .f_lock = __SPIN_LOCK_UNLOCKED(pcd_file.f_lock),
    .f_write_hint = 0,
    .f_count = ATOMIC_INIT(1),
    .f_flags = 0,
    .f_mode = 0,
    .f_pos_lock = __MUTEX_INITIALIZER(pcd_file.f_pos_lock),
    .f_inode = NULL,
    .f_op = NULL,            // Will be set later
    .f_ra = {0},             // Read-ahead state
    .f_pos = 0,              // Current position in the file
    .file_operations = NULL, // VFS intializes the f_ops field
    .f_owner = {0},
    .f_cred = NULL,
};
#endif

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
    alloc_chrdev_region(&device_number, 0, 1, DEVICE_NAME);

    if (device_number < 0)
    {
        printk(KERN_ALERT "Failed to allocate device number\n");
        return device_number;
    }
    printk(KERN_INFO "%s : Device number allocated - Major:Minor = %d, %d\n", __func__, MAJOR(device_number), MINOR(device_number));

    /* 2. Register the character device with VFS */
    /*  for multiple devices, we need to create cdev variable
        10 devices, we need to create 10 cdev variables         */

    cdev_init(&pcd_cdev, &pcd_fops); // Initialize the cdev with file operations
    pcd_cdev.owner = THIS_MODULE;    // Set the owner of the cdev
    pcd_cdev.dev = device_number;    // Set the device number for the cdev
    pcd_cdev.ops = &pcd_fops;        // Set the file operations for the cdev

    /* 3. Add the char device to the system for 1 file */
    if (cdev_add(&pcd_cdev, device_number, 1) < 0)
    {
        printk(KERN_ALERT "Failed to add cdev\n");
        unregister_chrdev_region(device_number, 1);
        return EXIT_FAILURE;
    }

    /* 4. Create Device class under /sys/class*/
    class_pcd = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(class_pcd)) // convert void pointer to struct class pointer
    {
        printk(KERN_ALERT "Failed to create device class\n");
        cdev_del(&pcd_cdev);                        // Remove the cdev
        unregister_chrdev_region(device_number, 1); // Unregister the device number
        return PTR_ERR(class_pcd);                  // convert void pointer to int pointer
    }
    printk(KERN_INFO "Device class created successfully\n");

    /* 5. Create Device under /dev - Populate the sysfs with device information*/
    device_pcd = device_create(class_pcd, NULL, device_number, NULL, DEVICE_NAME);
    if (IS_ERR(device_pcd))
    {
        printk(KERN_ALERT "Failed to create device\n");
        class_destroy(class_pcd);                   // Destroy the class
        cdev_del(&pcd_cdev);                        // Remove the cdev
        unregister_chrdev_region(device_number, 1); // Unregister the device number
        return PTR_ERR(device_pcd);
    }

    printk(KERN_INFO "Device created successfully: %s\n", DEVICE_NAME);

    // device_pcd->class = class_pcd; // Set the class for the device
    // device_pcd->devt = device_number; // Set the device number for the device

    printk(KERN_INFO "Pseudo Character Device Driver Initialization Complete\n");

    return EXIT_SUCCESS;
}

static void __exit pcd_driver_exit(void)
{
    printk(KERN_INFO "Pseudo Character Device Driver Exited\n");

    /* 1. Remove the device from sysfs */
    device_destroy(class_pcd, device_number);

    /* 2. Unregister the device class */
    class_destroy(class_pcd);

    /* 3. Remove the cdev from the system */
    cdev_del(&pcd_cdev);

    // cdev_deinit(&pcd_cdev); // Deinitialize the cdev
    //  Free the device number
    printk(KERN_INFO "Freeing device number: %d\n", MAJOR(device_number));
    // Note: The device number is freed automatically when the cdev is removed
    // If you want to explicitly free it, you can use unregister_chrdev_region
    // but it's not necessary here as we are already removing the cdev.

    /* 5. Unregister the character device region */
    unregister_chrdev_region(device_number, 1);

    printk(KERN_INFO "Pseudo Character Device Driver Cleanup Complete\n");
}

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
