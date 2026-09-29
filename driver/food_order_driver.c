#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/mutex.h>

#define DEVICE_NAME "food_order"
#define CLASS_NAME  "food"

#define BUFFER_SIZE 1024

static dev_t device_number;
static struct cdev food_cdev;
static struct class *food_class;
static struct device *food_device;

static char device_buffer[BUFFER_SIZE];
static size_t buffer_size;

static DEFINE_MUTEX(food_mutex);


/*
 * Called when /dev/food_order is opened.
 */
static int food_open(struct inode *inode, struct file *file)
{
    pr_info("food_order: device opened\n");
    return 0;
}


/*
 * Called when data is read from /dev/food_order.
 */
static ssize_t food_read(
    struct file *file,
    char __user *user_buffer,
    size_t count,
    loff_t *offset)
{
    ssize_t bytes_to_copy;

    if (*offset >= buffer_size)
        return 0;

    mutex_lock(&food_mutex);

    bytes_to_copy = min(count, buffer_size - (size_t)*offset);

    if (copy_to_user(
            user_buffer,
            device_buffer + *offset,
            bytes_to_copy))
    {
        mutex_unlock(&food_mutex);
        return -EFAULT;
    }

    *offset += bytes_to_copy;

    mutex_unlock(&food_mutex);

    pr_info(
        "food_order: read %zd bytes\n",
        bytes_to_copy
    );

    return bytes_to_copy;
}


/*
 * Called when data is written to /dev/food_order.
 */
static ssize_t food_write(
    struct file *file,
    const char __user *user_buffer,
    size_t count,
    loff_t *offset)
{
    size_t bytes_to_copy;

    bytes_to_copy = min(count, (size_t)(BUFFER_SIZE - 1));

    mutex_lock(&food_mutex);

    if (copy_from_user(
            device_buffer,
            user_buffer,
            bytes_to_copy))
    {
        mutex_unlock(&food_mutex);
        return -EFAULT;
    }

    device_buffer[bytes_to_copy] = '\0';
    buffer_size = bytes_to_copy;

    mutex_unlock(&food_mutex);

    pr_info(
        "food_order: received event: %s\n",
        device_buffer
    );

    return bytes_to_copy;
}


/*
 * Called when /dev/food_order is closed.
 */
static int food_release(
    struct inode *inode,
    struct file *file)
{
    pr_info("food_order: device closed\n");
    return 0;
}


static const struct file_operations food_fops = {
    .owner = THIS_MODULE,
    .open = food_open,
    .read = food_read,
    .write = food_write,
    .release = food_release,
};


/*
 * Module initialization.
 */
static int __init food_driver_init(void)
{
    int result;

    /*
     * Allocate a major/minor device number.
     */
    result = alloc_chrdev_region(
        &device_number,
        0,
        1,
        DEVICE_NAME
    );

    if (result < 0)
    {
        pr_err(
            "food_order: failed to allocate device number\n"
        );

        return result;
    }

    /*
     * Initialize character device.
     */
    cdev_init(
        &food_cdev,
        &food_fops
    );

    food_cdev.owner = THIS_MODULE;

    result = cdev_add(
        &food_cdev,
        device_number,
        1
    );

    if (result < 0)
    {
        unregister_chrdev_region(
            device_number,
            1
        );

        pr_err(
            "food_order: failed to add character device\n"
        );

        return result;
    }

    /*
     * Create device class.
     */
    food_class = class_create(
        CLASS_NAME
    );

    if (IS_ERR(food_class))
    {
        cdev_del(&food_cdev);

        unregister_chrdev_region(
            device_number,
            1
        );

        pr_err(
            "food_order: failed to create device class\n"
        );

        return PTR_ERR(food_class);
    }

    /*
     * Create /dev/food_order.
     */
    food_device = device_create(
        food_class,
        NULL,
        device_number,
        NULL,
        DEVICE_NAME
    );

    if (IS_ERR(food_device))
    {
        class_destroy(food_class);

        cdev_del(&food_cdev);

        unregister_chrdev_region(
            device_number,
            1
        );

        pr_err(
            "food_order: failed to create device\n"
        );

        return PTR_ERR(food_device);
    }

    pr_info(
        "food_order: driver loaded successfully\n"
    );

    pr_info(
        "food_order: device available at /dev/%s\n",
        DEVICE_NAME
    );

    return 0;
}


/*
 * Module cleanup.
 */
static void __exit food_driver_exit(void)
{
    device_destroy(
        food_class,
        device_number
    );

    class_destroy(food_class);

    cdev_del(&food_cdev);

    unregister_chrdev_region(
        device_number,
        1
    );

    pr_info(
        "food_order: driver unloaded\n"
    );
}


module_init(food_driver_init);
module_exit(food_driver_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Food Ordering System");
MODULE_DESCRIPTION(
    "Linux character device driver for food order events"
);
MODULE_VERSION("1.0");
