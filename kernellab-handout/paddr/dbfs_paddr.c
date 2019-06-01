#include <linux/debugfs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>
#include <asm/pgtable.h>

MODULE_LICENSE("GPL");

static struct dentry *dir, *output;
static struct task_struct *task;

struct packet {
	pid_t pid;
	unsigned long vaddr, paddr;
};

// Calculates offset
unsigned long offset(unsigned long entry) {
	// mask out lower 12 bits and add offset
	return (entry & 0xFFFFFFFFFF000) + PAGE_OFFSET;
}

static ssize_t read_output(struct file *fp,
		char __user *user_buffer, size_t length, loff_t *position) {

	// Implement read file operation
	struct packet *pckt; // packet
	pgd_t *pgd; // page global directory
	p4d_t *p4d; // ?
	pud_t *pud; // page upper directory
	pmd_t *pmd; // page middle directory
	pte_t *pte; // page table entry / page table itself

	struct page* pg;

	pckt = (struct packet*) user_buffer;

	task = pid_task(find_get_pid(pckt -> pid), PIDTYPE_PID); // get task_struct

	// get pgd from task -> mm
	pgd = pgd_offset(task -> mm, pckt -> vaddr);

	// get p4d from pgd
	p4d = p4d_offset(pgd, pckt -> vaddr);

	// get pud from p4d
	pud = pud_offset(p4d, pckt -> vaddr);

	// get pmd from pud
	pmd = pmd_offset(pud, pckt -> vaddr);

	// get pte from pmd
	pte = pte_offset_map(pmd, pckt -> vaddr);

	// get page
	pg = pte_page(*pte);

	// map page to physical address
	pckt -> paddr = page_to_phys(pg);
	pte_unmap(pte);

	return 0;
}

static const struct file_operations dbfs_fops = {
	// Mapping file operations with your functions
	.read = read_output,
};

static int __init dbfs_module_init(void) {
	// Nothing much to do on init
	// Implement init module
	dir = debugfs_create_dir("paddr", NULL);
	if (!dir) {
		printk("Cannot create paddr dir\n");
		return -1;
	}

	// Fill in the arguments below
	output = debugfs_create_file("output", S_IWUSR, dir, NULL, &dbfs_fops);

	printk("dbfs_paddr module initialize done\n");
	return 0;
}

static void __exit dbfs_module_exit(void) {
	// Implement exit module code
	printk("dbfs_paddr module exit\n");
	debugfs_remove_recursive(dir);
}

module_init(dbfs_module_init);
module_exit(dbfs_module_exit);
