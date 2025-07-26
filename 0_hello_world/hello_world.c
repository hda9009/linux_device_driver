#include <linux/module.h>
#include <linux/kernel.h>

static int __init hello_world_init(void)
{
    printk(KERN_INFO "Hello, World!\n");
    return 0; // Return 0 indicates successful initialization
}

static void __exit hello_world_exit(void)
{
    printk(KERN_INFO "Goodbye, World!\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("HDA");
MODULE_DESCRIPTION("A simple Hello World LKM");
MODULE_INFO(board, "BeagleBoard AM335x");