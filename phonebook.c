#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/string.h>

#define DEVICE_NAME "phonebook"
#define CLASS_NAME "phonebook_class"
#define MAX_USERS 100

static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);

static int major_number;
static struct class* phonebook_class = NULL;
static struct device* phonebook_device = NULL;
static DEFINE_MUTEX(phonebook_mutex);

struct User {
    char first_name[50];
    char last_name[50];
    int age;
    char phone_number[15];
    char email[50];
};

static struct User users[MAX_USERS];
static int user_count = 0;

long add_phonebook_user(struct User* input_data);
long del_phonebook_user(const char* first_name, const char* last_name);
long get_phonebook_user(const char* surname, unsigned int len, struct User* output_data);

static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write,
};

static int device_open(struct inode* inodep, struct file* filep) {
    if (!mutex_trylock(&phonebook_mutex)) {
        printk(KERN_ALERT "Phonebook: Device is being used by another process\n");
        return -EBUSY;
    }
    printk(KERN_INFO "Phonebook: Device opened\n");
    return 0;
}

static int device_release(struct inode* inodep, struct file* filep) {
    mutex_unlock(&phonebook_mutex);
    printk(KERN_INFO "Phonebook: Device successfully closed\n");
    return 0;
}

static ssize_t device_read(struct file* filep, char* buffer, size_t len, loff_t* offset) {
    struct User output_data;
    static int current_user_index = 0;

    if (current_user_index >= user_count) {
        current_user_index = 0;
        return 0;
    }

    output_data = users[current_user_index];
    current_user_index++;

    int ret;
    int bytes_read = 0;
    char output_buffer[1024];
    snprintf(output_buffer, sizeof(output_buffer), "Name: %s %s\nAge: %d\nPhone: %s\nEmail: %s\n\n",
             output_data.first_name, output_data.last_name,
             output_data.age, output_data.phone_number, output_data.email);

    ret = copy_to_user(buffer, &output_buffer, sizeof(output_buffer));
    if (ret) {
        printk(KERN_ERR "Phonebook: Failed to copy User data to User space\n");
        return -EFAULT;
    }

    bytes_read = sizeof(output_buffer);

    return bytes_read;
}

long add_phonebook_user(struct User* input_data) {
    if (user_count >= MAX_USERS) {
        printk(KERN_ALERT "Phonebook: Maximum number of users reached\n");
        return -ENOMEM;
    }

    users[user_count] = *input_data;
    user_count++;
    printk(KERN_INFO "Phonebook: Added User %s %s\n", input_data->first_name, input_data->last_name);
    return 0;
}

long del_phonebook_user(const char* first_name, const char* last_name) {
    int i;
    for (i = 0; i < user_count; i++) {
        if (strncmp(users[i].first_name, first_name, 50) == 0 &&
            strncmp(users[i].last_name, last_name, 50) == 0) {
            printk(KERN_INFO "Phonebook: Deleting User %s %s\n", users[i].first_name, users[i].last_name);
            users[i] = users[user_count - 1];
            user_count--;
            return 0;
        }
    }

    printk(KERN_INFO "Phonebook: User %s %s not found\n", first_name, last_name);
    return -EINVAL;
}

long get_phonebook_user(const char* surname, unsigned int len, struct User* output_data) {
    int i;

    for (i = 0; i < user_count; i++) {
        if (strncmp(users[i].last_name, surname, len) == 0) {
            *output_data = users[i];
            return 0;
        }
    }

    return -EINVAL;
}

static ssize_t device_write(struct file* filep, const char* buffer, size_t len, loff_t* offset) {
    char command[256];
    char user_first_name[50], user_last_name[50], user_phone[15], user_email[50];
    int user_age;
    struct User new_user;
    struct User found_user;

    if (len > sizeof(command) - 1) {
        return -EINVAL;
    }

    if (copy_from_user(command, buffer, len)) {
        return -EFAULT;
    }

    command[len] = '\0';

    if (sscanf(command, "add_user %49s %49s %d %14s %49s", user_first_name, user_last_name, &user_age, user_phone, user_email) == 5) {
        strncpy(new_user.first_name, user_first_name, sizeof(new_user.first_name) - 1);
        new_user.first_name[sizeof(new_user.first_name) - 1] = '\0';

        strncpy(new_user.last_name, user_last_name, sizeof(new_user.last_name) - 1);
        new_user.last_name[sizeof(new_user.last_name) - 1] = '\0';

        new_user.age = user_age;

        strncpy(new_user.phone_number, user_phone, sizeof(new_user.phone_number) - 1);
        new_user.phone_number[sizeof(new_user.phone_number) - 1] = '\0';

        strncpy(new_user.email, user_email, sizeof(new_user.email) - 1);
        new_user.email[sizeof(new_user.email) - 1] = '\0';

        if (add_phonebook_user(&new_user) != 0) {
            return -EINVAL;
        }

        return len;
    }

    if (sscanf(command, "del_user %49s %49s", user_first_name, user_last_name) == 2) {
        if (del_phonebook_user(user_first_name, user_last_name) != 0) {
            return -EINVAL;
        }

        return len;
    }

    if (sscanf(command, "get_user %49s", user_last_name) == 1) {
        if (get_phonebook_user(user_last_name, strlen(user_last_name), &found_user) != 0) {
            printk(KERN_WARNING "Phonebook: User with surname %s not found\n", user_last_name);
            return -EINVAL;
        }

        printk(KERN_INFO "Phonebook: Found User: %s %s, Age: %d, Phone: %s, Email: %s\n",
               found_user.first_name, found_user.last_name, found_user.age,
               found_user.phone_number, found_user.email);

        return len;
    }
    
    printk(KERN_WARNING "Phonebook: Invalid command format\n");
    return -EINVAL;
}

static int __init phonebook_init(void) {
    printk(KERN_INFO "Phonebook: Initializing phonebook module\n");

    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ALERT "Phonebook failed to register a major number\n");
        return major_number;
    }

    phonebook_class = class_create(CLASS_NAME);
    if (IS_ERR(phonebook_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(phonebook_class);
    }

    phonebook_device = device_create(phonebook_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(phonebook_device)) {
        class_destroy(phonebook_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(phonebook_device);
    }

    mutex_init(&phonebook_mutex);
    printk(KERN_INFO "Phonebook: Device created successfully\n");
    return 0;
}

static void __exit phonebook_exit(void) {
    mutex_destroy(&phonebook_mutex);
    device_destroy(phonebook_class, MKDEV(major_number, 0));
    class_unregister(phonebook_class);
    class_destroy(phonebook_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Phonebook: Module exited successfully\n");
}

module_init(phonebook_init);
module_exit(phonebook_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MARAT AMIROV");
MODULE_DESCRIPTION("Phonebook kernel module that stores user info");