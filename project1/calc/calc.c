#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#define MAX_SIZE 128
#define ID "521021910653"

// Input from caller
static int operand1;
module_param(operand1, int, 0);
static char *operator;
module_param(operator, charp, 0);
static int operand2[MAX_SIZE];
static int ninp;
module_param_array(operand2, int, &ninp, 0);

static struct proc_dir_entry *proc_ent;
static struct proc_dir_entry *proc_dir;
static char output[MAX_SIZE];
int out_len;

static ssize_t proc_read(struct file *fp, char __user *ubuf, size_t len, loff_t *pos)
{
    /* TODO */
    out_len = 0;
    if (*pos > 0) {
        return 0;
    } 
    if (len < MAX_SIZE) {
        return -EFAULT;
    }
    // Compare the operator with "add"
    // Perform the corresponding operation
    int i;
    if (strcmp(operator, "add") == 0) {
        for (i = 0; i < ninp; ++i) {
            int cur = operand2[i] + operand1;
            out_len += snprintf(output + out_len, MAX_SIZE - out_len, "%d,", cur);
        }
    } else if (strcmp(operator, "mul") == 0) {
        for (i = 0; i < ninp; ++i) {
            int cur = operand2[i] * operand1;
            out_len += snprintf(output + out_len, MAX_SIZE - out_len, "%d,", cur);
        }
    } else {
        printk(KERN_ERR "Invalid operator\n");
        return -EFAULT;
    }
    printk(KERN_INFO "Finish calculating\n");
    // Write the value out
    output[out_len - 1] = '\n';
    output[out_len] = '\0';
    if (copy_to_user(ubuf, output, out_len)) {
        printk(KERN_ERR "Error: Failed to copy to user buffer\n");
        return -EFAULT;
    }
    printk(KERN_INFO "Successfully copy %d to user buffer\n", out_len);
    *pos = out_len;
    return out_len;
}

static ssize_t proc_write(struct file *fp, const char __user *ubuf, size_t len, loff_t *pos)
{
    /* TODO */
    if (*pos > 0 || len > MAX_SIZE) {
        printk(KERN_ERR "Error: Invalid parameter\n");
        return -EFAULT;
    }
    // Read the data in
    if (copy_from_user(output, ubuf, len)) {
        printk(KERN_ERR "Error: Failed to copy user buffer into kernel buffer\n");
        return -EFAULT;
    }
    printk(KERN_INFO "Successfully copy from user buffer\n");
    output[len] = '\0';
    // To integer and Update operand
    if (kstrtoint(output, 10, &operand1)) {
        printk(KERN_ERR "Error: Failed to convert string into integer\n");
        return -EFAULT;
    }
    printk(KERN_INFO "New operand is %d\n", operand1);
    return len;
}

static const struct proc_ops proc_ops = {
    .proc_read = proc_read,
    .proc_write = proc_write,
};

static int __init proc_init(void)
{
    /* TODO */
    proc_dir = proc_mkdir(ID, NULL);
    if (NULL == proc_dir) {
        proc_remove(proc_dir);
        printk(KERN_ALERT "Error: Can not initialize /proc/$ID\n");
        return -ENOMEM;
    }
    proc_ent = proc_create("calc", 0644, proc_dir, &proc_ops);
    if (NULL == proc_ent) {
        proc_remove(proc_ent);
        printk(KERN_ALERT "Error: Can not initialize /proc/$ID/calc\n");
        return -ENOMEM;
    }
    printk(KERN_INFO "/proc/$ID/calc created\n");
    return 0;
}

static void __exit proc_exit(void)
{
    /* TODO */
    proc_remove(proc_ent);
    printk(KERN_INFO "/proc/$ID/calc removed\n");
    proc_remove(proc_dir);
    printk(KERN_INFO "/proc/$ID removed\n");    
}

module_init(proc_init);
module_exit(proc_exit);
MODULE_LICENSE("GPL");
