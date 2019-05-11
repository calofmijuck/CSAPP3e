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
	pud_t *pud; // page upper directory
	pmd_t *pmd; // page middle directory
	pte_t *pte; // page table entry / page table itself

	// virtual page number for each table level, virtual page offset
	unsigned long vpn1, vpn2, vpn3, vpn4, vpo;
	unsigned long vaddr_tmp;

	pckt = (struct packet*) user_buffer;

	vaddr_tmp = pckt -> vaddr;

	// mask out addresses
	vpo = vaddr_tmp & 0xFFF; // mask out last 12 bits
	vpn4 = (vaddr_tmp >>= 12) & 0x1FF; // next 9 bits for each levels
	vpn3 = (vaddr_tmp >>= 9) & 0x1FF;
	vpn2 = (vaddr_tmp >>= 9) & 0x1FF;
	vpn1 = (vaddr_tmp >>= 9) & 0x1FF;

	task = pid_task(find_get_pid(pckt -> pid), PIDTYPE_PID); // get task_struct

	pgd = task -> mm -> pgd; // get mm_struct and first page table

	// get location of pud from pgd
	pud = (pud_t *) offset((pgd + vpn1) -> pgd);
	// get location of pmd from pud
	pmd = (pmd_t *) offset((pud + vpn2) -> pud);
	// get location of pte from pmd
	pte = (pte_t *) offset((pmd + vpn3) -> pmd);
	// get physical address from pte and use virtual page offset here
	pckt -> paddr = ((pte + vpn4) -> pte & 0xFFFFFFFFFF000) + vpo;
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

	// printk("dbfs_paddr module initialize done\n");
	return 0;
}

static void __exit dbfs_module_exit(void) {
	// Implement exit module code
	// printk("dbfs_paddr module exit\n");
}

module_init(dbfs_module_init);
module_exit(dbfs_module_exit);
