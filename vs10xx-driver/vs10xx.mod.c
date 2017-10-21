#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xb344870e, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xa20197bc, __VMLINUX_SYMBOL_STR(cdev_alloc) },
	{ 0x5f3aede0, __VMLINUX_SYMBOL_STR(cdev_del) },
	{ 0xfd3e178c, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xaebc5af3, __VMLINUX_SYMBOL_STR(cdev_init) },
	{ 0xf9a482f9, __VMLINUX_SYMBOL_STR(msleep) },
	{ 0xff178f6, __VMLINUX_SYMBOL_STR(__aeabi_idivmod) },
	{ 0x1494ad7b, __VMLINUX_SYMBOL_STR(param_ops_int) },
	{ 0x2e5810c6, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr1) },
	{ 0xcd387ee9, __VMLINUX_SYMBOL_STR(gpio_to_desc) },
	{ 0x18391aa, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0xffaa9a00, __VMLINUX_SYMBOL_STR(of_property_read_u32_array) },
	{ 0x83e2fe2b, __VMLINUX_SYMBOL_STR(__spi_register_driver) },
	{ 0xb1ad28e0, __VMLINUX_SYMBOL_STR(__gnu_mcount_nc) },
	{ 0x28cc25db, __VMLINUX_SYMBOL_STR(arm_copy_from_user) },
	{ 0x5c7615b7, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0x7485e15e, __VMLINUX_SYMBOL_STR(unregister_chrdev_region) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0x7d11c268, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0xf4fa543b, __VMLINUX_SYMBOL_STR(arm_copy_to_user) },
	{ 0x275ef902, __VMLINUX_SYMBOL_STR(__init_waitqueue_head) },
	{ 0xfa2a45e, __VMLINUX_SYMBOL_STR(__memzero) },
	{ 0x5f754e5a, __VMLINUX_SYMBOL_STR(memset) },
	{ 0xeaad5b7a, __VMLINUX_SYMBOL_STR(__mutex_init) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x1403bc31, __VMLINUX_SYMBOL_STR(sysfs_create_group) },
	{ 0x919edf7c, __VMLINUX_SYMBOL_STR(class_unregister) },
	{ 0x79c5a9f0, __VMLINUX_SYMBOL_STR(ioremap) },
	{ 0xeec4978e, __VMLINUX_SYMBOL_STR(driver_unregister) },
	{ 0xa1c76e0a, __VMLINUX_SYMBOL_STR(_cond_resched) },
	{ 0x98211578, __VMLINUX_SYMBOL_STR(mutex_lock) },
	{ 0x1490a67c, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0xd6b8e852, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0x8e865d3c, __VMLINUX_SYMBOL_STR(arm_delay_ops) },
	{ 0x3757e6ad, __VMLINUX_SYMBOL_STR(__class_register) },
	{ 0x2196324, __VMLINUX_SYMBOL_STR(__aeabi_idiv) },
	{ 0xe523ad75, __VMLINUX_SYMBOL_STR(synchronize_irq) },
	{ 0x3bff2ea, __VMLINUX_SYMBOL_STR(spi_sync) },
	{ 0x5aa02962, __VMLINUX_SYMBOL_STR(cdev_add) },
	{ 0xd62c833f, __VMLINUX_SYMBOL_STR(schedule_timeout) },
	{ 0x71005c25, __VMLINUX_SYMBOL_STR(of_get_property) },
	{ 0x566df243, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xd85cd67e, __VMLINUX_SYMBOL_STR(__wake_up) },
	{ 0x344b7739, __VMLINUX_SYMBOL_STR(prepare_to_wait_event) },
	{ 0xfe990052, __VMLINUX_SYMBOL_STR(gpio_free) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x14b94ab8, __VMLINUX_SYMBOL_STR(gpiod_to_irq) },
	{ 0x64f8b539, __VMLINUX_SYMBOL_STR(gpiod_set_raw_value) },
	{ 0x1cfb04fa, __VMLINUX_SYMBOL_STR(finish_wait) },
	{ 0x7f02188f, __VMLINUX_SYMBOL_STR(__msecs_to_jiffies) },
	{ 0x29537c9e, __VMLINUX_SYMBOL_STR(alloc_chrdev_region) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("of:N*T*Cvs1001-ctrl*");
MODULE_ALIAS("of:N*T*Cvs1011-ctrl*");
MODULE_ALIAS("of:N*T*Cvs1002-ctrl*");
MODULE_ALIAS("of:N*T*Cvs1003-ctrl*");
MODULE_ALIAS("of:N*T*Cvs1053-ctrl*");
MODULE_ALIAS("of:N*T*Cvs1033-ctrl*");
MODULE_ALIAS("of:N*T*Cvs1103-ctrl*");
MODULE_ALIAS("spi:vs1001-data");
MODULE_ALIAS("spi:vs1011-data");
MODULE_ALIAS("spi:vs1002-data");
MODULE_ALIAS("spi:vs1003-data");
MODULE_ALIAS("spi:vs1053-data");
MODULE_ALIAS("spi:vs1033-data");
MODULE_ALIAS("spi:vs1103-data");
MODULE_ALIAS("of:N*T*Cvs1001-data*");
MODULE_ALIAS("of:N*T*Cvs1011-data*");
MODULE_ALIAS("of:N*T*Cvs1002-data*");
MODULE_ALIAS("of:N*T*Cvs1003-data*");
MODULE_ALIAS("of:N*T*Cvs1053-data*");
MODULE_ALIAS("of:N*T*Cvs1033-data*");
MODULE_ALIAS("of:N*T*Cvs1103-data*");
MODULE_ALIAS("spi:vs1001-ctrl");
MODULE_ALIAS("spi:vs1011-ctrl");
MODULE_ALIAS("spi:vs1002-ctrl");
MODULE_ALIAS("spi:vs1003-ctrl");
MODULE_ALIAS("spi:vs1053-ctrl");
MODULE_ALIAS("spi:vs1033-ctrl");
MODULE_ALIAS("spi:vs1103-ctrl");

MODULE_INFO(srcversion, "2FB98A3487372D603A82511");
