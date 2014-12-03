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
	{ 0x561bc67, "unregister_netdev" },
	{ 0x37a0cba, "kfree" },
	{ 0xe42cc6e1, "register_netdev" },
	{ 0xa272a4b0, "alloc_etherdev_mqs" },
	{ 0xb703ea3c, "__netif_schedule" },
	{ 0x6194b31c, "consume_skb" },
	{ 0x42202b82, "netif_rx" },
	{ 0x6d3426cd, "eth_type_trans" },
	{ 0x1795e732, "skb_put" },
	{ 0x4f89ee0c, "dev_alloc_skb" },
	{ 0x50eedeb8, "printk" },
	{ 0x16305289, "warn_slowpath_null" },
	{ 0x2e60bace, "memcpy" },
	{ 0x8321d7ed, "skb_push" },
	{ 0xb4390f9a, "mcount" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "8E01864768EEDFAD60DDC4C");
