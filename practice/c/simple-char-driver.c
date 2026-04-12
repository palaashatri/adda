// simple-char-driver.c -> for Linux. This is a simple character driver that initializes and exits cleanly, printing messages to the kernel log.
// To compile this driver, you would typically create a Makefile and use the `make` command to build it against the Linux kernel headers.
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int __init simple_char_driver_init(void) {
    printk(KERN_INFO "Simple Char Driver: Initializing\n");
    return 0; // Return 0 for successful initialization
}

static void __exit simple_char_driver_exit(void) {
    printk(KERN_INFO "Simple Char Driver: Exiting\n");
}

module_init(simple_char_driver_init);
module_exit(simple_char_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Palaash Atri");
MODULE_DESCRIPTION("Simple Char Driver");
MODULE_VERSION("0.1");

