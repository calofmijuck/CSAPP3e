#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#define BUF 101010

MODULE_LICENSE("GPL");

int size;
static struct dentry *dir, *inputdir, *ptreedir;
static struct task_struct *curr;
static struct debugfs_blob_wrapper blob;

void print_pid(struct task_struct* task) {
	if(task ->  pid > 1) print_pid(task -> real_parent); // if not init, recursive call
	size = snprintf(blob.data + blob.size, BUF - blob.size, "%s (%d)\n", task -> comm, task -> pid); // print to buffer
	blob.size += size;
}

static ssize_t write_pid_to_input(struct file *fp, const char __user *user_buffer, size_t length, loff_t *position) {
	pid_t input_pid;
	sscanf(user_buffer, "%u", &input_pid); // get input
	curr = pid_task(find_get_pid(input_pid), PIDTYPE_PID); // find task struct from input_pid
	blob.size = 0; // initialize blob size
	print_pid(curr); // print pid recursively
	return length;
}

static const struct file_operations dbfs_fops = {
	.write = write_pid_to_input,
};

static int __init dbfs_module_init(void) { // will be executed on initialization
	static char buf[BUF]; // buffer to print to
	blob.data = buf; // set data to buf

	// Implement init module code
	dir = debugfs_create_dir("ptree", NULL);
	if (!dir) {
		printk("Cannot create ptree dir\n");
		return -1;
	}

	// struct dentry *debugfs_create_file(const char *name, umode_t mode,
	//									  struct dentry *parent, void *data,
	//                                    const struct file_operations *fops)
	inputdir = debugfs_create_file("input", S_IWUSR, dir, NULL, &dbfs_fops);

	// struct dentry *debugfs_create_blob(const char *name, umode_t mode,
	//								  	  sturct dentry *parent,
	//                                    struct debugfs_blob_wrapper *blob)
	ptreedir = debugfs_create_blob("ptree", S_IRUSR, dir, &blob);
	// printk("dbfs_ptree module initialize done\n");
	return 0;
}

static void __exit dbfs_module_exit(void) { // executed on exit
	// Implement exit module code
	// printk("dbfs_ptree module exit\n");
}

module_init(dbfs_module_init);
module_exit(dbfs_module_exit);
