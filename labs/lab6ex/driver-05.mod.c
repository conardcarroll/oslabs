#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
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
	{ 0x5eadf54a, "module_layout" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x7485e15e, "unregister_chrdev_region" },
	{ 0xb714e933, "cdev_del" },
	{ 0x48eb0c0d, "__init_waitqueue_head" },
	{ 0x657879ce, "__init_rwsem" },
	{ 0x9b604f4c, "cdev_add" },
	{ 0x29537c9e, "alloc_chrdev_region" },
	{ 0x2563f926, "cdev_alloc" },
	{ 0x72df2f2a, "up_read" },
	{ 0x2f287f0d, "copy_to_user" },
	{ 0xd0f0d945, "down_read" },
	{ 0xbc1afedf, "up_write" },
	{ 0x362ef408, "_copy_from_user" },
	{ 0x75bb675a, "finish_wait" },
	{ 0xd62c833f, "schedule_timeout" },
	{ 0x622fa02a, "prepare_to_wait" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0x131ff057, "current_task" },
	{ 0x61b5ade0, "down_write" },
	{ 0x50eedeb8, "printk" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "4E0447508E9881D683AC232");
