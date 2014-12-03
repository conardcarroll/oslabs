cmd_/vagrant/labs/lab5/07-prod-cons.o := gcc -Wp,-MD,/vagrant/labs/lab5/.07-prod-cons.o.d  -nostdinc -isystem /usr/lib/gcc/i686-linux-gnu/4.6/include  -I/usr/src/linux-headers-3.2.0-23-generic-pae/arch/x86/include -Iarch/x86/include/generated -Iinclude  -include /usr/src/linux-headers-3.2.0-23-generic-pae/include/linux/kconfig.h -Iubuntu/include  -D__KERNEL__ -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -Wno-format-security -fno-delete-null-pointer-checks -O2 -m32 -msoft-float -mregparm=3 -freg-struct-return -mpreferred-stack-boundary=2 -march=i686 -mtune=generic -maccumulate-outgoing-args -Wa,-mtune=generic32 -ffreestanding -fstack-protector -DCONFIG_AS_CFI=1 -DCONFIG_AS_CFI_SIGNAL_FRAME=1 -DCONFIG_AS_CFI_SECTIONS=1 -pipe -Wno-sign-compare -fno-asynchronous-unwind-tables -mno-sse -mno-mmx -mno-sse2 -mno-3dnow -Wframe-larger-than=1024 -Wno-unused-but-set-variable -fno-omit-frame-pointer -fno-optimize-sibling-calls -pg -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fconserve-stack -DCC_HAVE_ASM_GOTO  -DMODULE  -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(07_prod_cons)"  -D"KBUILD_MODNAME=KBUILD_STR(07_prod_cons)" -c -o /vagrant/labs/lab5/.tmp_07-prod-cons.o /vagrant/labs/lab5/07-prod-cons.c

source_/vagrant/labs/lab5/07-prod-cons.o := /vagrant/labs/lab5/07-prod-cons.c

deps_/vagrant/labs/lab5/07-prod-cons.o := \

/vagrant/labs/lab5/07-prod-cons.o: $(deps_/vagrant/labs/lab5/07-prod-cons.o)

$(deps_/vagrant/labs/lab5/07-prod-cons.o):
