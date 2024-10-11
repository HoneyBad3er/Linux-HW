#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/atomic.h>


const static int IRQ = 1;
const static int INTERVAL = 60;
static atomic_t key_press_count = ATOMIC_INIT(0);
static int stop_timer = 0;
static struct timer_list print_timer;


static irqreturn_t keypress_handler(int irq, void *dev) {
    atomic_inc(&key_press_count);
    return IRQ_HANDLED;
}


void print_keypress_count(struct timer_list *timer) {
    printk(KERN_INFO "KeyPressCounter: keypresses cnt = %d\n", atomic_read(&key_press_count));

    if (!stop_timer) {
        mod_timer(&print_timer, jiffies + INTERVAL * HZ);
    }
}


static int __init keycounter_init(void) {
    int ret = request_irq(IRQ, keypress_handler, IRQF_SHARED, "keyboard_irq_handler", (void*)keypress_handler);
    if (ret) {
        printk(KERN_ERR "KeyPressCounter: Failed to register IRQ handler (error code: %d)\n", ret);
        return ret;
    }

    timer_setup(&print_timer, print_keypress_count, 0);
    mod_timer(&print_timer, jiffies + INTERVAL * HZ);

    printk(KERN_INFO "KeyPressCounter module initialized\n");
    return 0;
}


static void __exit keycounter_exit(void) {
    stop_timer = 1;

    synchronize_irq(IRQ);
    free_irq(IRQ, (void*)keypress_handler);

    del_timer_sync(&print_timer);

    printk(KERN_INFO "KeyPressCounter module exited\n");
}

module_init(keycounter_init);
module_exit(keycounter_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MARAT AMIROV");
MODULE_DESCRIPTION("KeyPressCounter kernel module that counts keypresses for PS/2 kb");
