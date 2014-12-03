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
	{ 0xd5f2172f, "del_timer_sync" },
	{ 0x8c03d20c, "destroy_workqueue" },
	{ 0x42160169, "flush_workqueue" },
	{ 0x47c149ab, "queue_delayed_work" },
	{ 0xfb0e29f, "init_timer_key" },
	{ 0x83699014, "kmem_cache_alloc_trace" },
	{ 0x85e90336, "kmalloc_caches" },
	{ 0x88a193, "__alloc_workqueue_key" },
	{ 0x50eedeb8, "printk" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "36DFDF8F47BB59B9145DEE7");
