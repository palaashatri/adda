// lkm_procinfo.c
// Linux Kernel Module: Hello World + Process/Memory Introspection
// Author : Palaash Atri  |  BITS ID: 2025NS01017
 
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/mm.h>
#include <linux/mm_types.h>
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Palaash Atri");
MODULE_DESCRIPTION("LKM: Hello World + Process/Memory Introspection");
MODULE_VERSION("1.0");
 
#define PROC_NAME "lkm_procinfo"
 
/* ----------------------------------------------------------
 * seq_show: called once per /proc read by the seq_file layer.
 * Traverses task_struct list; reads mm_struct for memory.
 * ---------------------------------------------------------- */
static int lkm_show(struct seq_file *m, void *v)
{
    struct task_struct *task;
 
    seq_printf(m, "%-8s %-20s %-6s %-16s %-16s\n",
               "PID", "NAME", "STATE", "VmSize(kB)", "VmRSS(kB)");
    seq_printf(m, "%-8s %-20s %-6s %-16s %-16s\n",
               "--------", "--------------------",
               "------", "----------------", "----------------");
 
    rcu_read_lock();
 
    for_each_process(task) {
        unsigned long vm_size_kb = 0;
        unsigned long vm_rss_kb  = 0;
 
        /*
         * Kernel threads have mm == NULL (no user address space).
         * Guard to avoid NULL-pointer dereference.
         */
        if (task->mm) {
            vm_size_kb = (task->mm->total_vm) << (PAGE_SHIFT - 10);
            vm_rss_kb  = (get_mm_rss(task->mm)) << (PAGE_SHIFT - 10);
        }
 
        seq_printf(m, "%-8d %-20s %-6c %-16lu %-16lu\n",
                   task->pid,
                   task->comm,
                   task_state_to_char(task),
                   vm_size_kb,
                   vm_rss_kb);
    }
 
    rcu_read_unlock();
    return 0;
}
 
static int lkm_open(struct inode *inode, struct file *file)
{
    return single_open(file, lkm_show, NULL);
}
 
static const struct proc_ops lkm_proc_ops = {
    .proc_open    = lkm_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};
 
/* ----------------------------------------------------------
 * Module init: print hello, create /proc entry
 * ---------------------------------------------------------- */
static int __init lkm_init(void)
{
    pr_info("[lkm_procinfo] Module loaded - Hello from Palaash Atri (2025NS01017)!\n");
 
    if (!proc_create(PROC_NAME, 0444, NULL, &lkm_proc_ops)) {
        pr_err("[lkm_procinfo] Failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }
 
    pr_info("[lkm_procinfo] /proc/%s created successfully.\n", PROC_NAME);
    return 0;
}
 
/* ----------------------------------------------------------
 * Module exit: remove /proc entry, print goodbye
 * ---------------------------------------------------------- */
static void __exit lkm_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);
    pr_info("[lkm_procinfo] Module unloaded - Goodbye from Palaash Atri (2025NS01017)!\n");
}
 
module_init(lkm_init);
module_exit(lkm_exit);
