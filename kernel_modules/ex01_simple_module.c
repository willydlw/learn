/** Build instructions
 * 
 * make -C /lib/modules/$(uname -r)/build M=$PWD modules
 * 
 * -C dir Change to directory dir before reading the makefiles or doing anything else.
 * M=$PWD says to use the makefile is working directory 
 * modules means build the module?
 * 
 * To insert the module into the kernel space
 * 
 * sudo insmod ./ex01_simple_module.ko
 * 
 * To confirm the module has been loaded into the kernel space
 * 
 * lsmod
 * 
 * or lsmod | grep "ex01"
 * 
 * 
 * To remove the module: sudo rmmod ex01_simple_module
 * 
 * To see messages added to the system log
 * sudo tail -f /var/log/syslog
 */

#include <linux/init.h>
#include <linux/module.h>

int ex01_simple_module_init(void)
{
    printk(KERN_ALERT "inside function %s\n", __FUNCTION__);
    return 0;
}

void ex01_simple_module_exit(void)
{
    printk(KERN_ALERT "function %s\n", __FUNCTION__);
}

/* register function to be called when module is loaded */
module_init(ex01_simple_module_init);

/* register function to be called when module is removed */
module_exit(ex01_simple_module_exit);