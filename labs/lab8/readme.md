# Lab 8
## Remarks
The implemenation of the buddy system is known, the linux kernel is already doing it. To see some of the
information just look in /proc/buddyinfo. Unfortunately even trying to use max's existing buddy allocator
proved to be a fairly difficult task. I'm sure if I wasn't finishing these labs with such a looming deadline
I would have had the time to ask more questions about composing linux device drivers. I understand that the buddy
allocator is a suitable memory allocation scheme and has been used appropriately many times in practice. It has the
advantage of lesser fragmentation between its pages but can still fall victim to fragmentation within its pages based
on the requests for allocation from other programs. While writing this code I've thought with IOCTL as the hammer 
everything becomes a nail. I wish I could have found a more elegant solution using the write/read methods but I could not.
I was able to get the write/read to act correctly, at least as far of the test program in the lab document exercised my code.

I started with driver-07 and attempted to make the read/write methods work, as opposed to the straight ioctl
calls I ended up with.  At one point I was able to get the read function to at least be called but was never able to
have the write function be called and anytime I called write() from user land it always returned -1. So I leaned a
bit more on Max and used his IOCTL scheme replacing the get_user and put_user with copy_from_user and copy_to_user. This proved
interesing trying to capture and know where the null string terminator was in order to save and return it. I'm sure there is 
a function or macro which would have done this for me. 

Running the test code from the lab document does 

##Results

###make

*vagrant@precise32:/vagrant/labs/lab8$ make*
make: Warning: File `Makefile' has modification time 8.7e+02 s in the future
make -C /lib/modules/3.2.0-23-generic-pae/build M=/vagrant/labs/lab8 modules
make[1]: Entering directory `/usr/src/linux-headers-3.2.0-23-generic-pae'
make[2]: Warning: File `/vagrant/labs/lab8/Makefile' has modification time 8.7e+02 s in the future
make[2]: warning:  Clock skew detected.  Your build may be incomplete.
  Building modules, stage 2.
make[2]: Warning: File `/vagrant/labs/lab8/Makefile' has modification time 8.7e+02 s in the future
  MODPOST 1 modules
make[2]: warning:  Clock skew detected.  Your build may be incomplete.
make[1]: Leaving directory `/usr/src/linux-headers-3.2.0-23-generic-pae'
make: warning:  Clock skew detected.  Your build may be incomplete.
*vagrant@precise32:/vagrant/labs/lab8$ make mem_test*
make: Warning: File `Makefile' has modification time 8.7e+02 s in the future
make: `mem_test' is up to date.
make: warning:  Clock skew detected.  Your build may be incomplete.

_clock skew warnings just happen using synced folders with virtual box_

###load driver

*vagrant@precise32:/vagrant/labs/lab8$ ./load.sh*

<6>[ 4062.633117] Registration is a success The major device number is 111.
[ 4062.633121] mem: Module loaded successfully (device number: 111, pool size: 4096)
vagrant@precise32:/vagrant/labs/lab8$

### run test

*vagrant@precise32:/vagrant/labs/lab8$ ./mem_test*
write_mem mem 3 ref 0 buf *Hello buddy* len 11
set index 0
wrote 0
set index 0
set read size 0
read 0 bytes
*buffer: lo buddy*

*vagrant@precise32:/vagrant/labs/lab8$ dmesg*

<6>[ 4240.578259] Registration is a success The major device number is 111.
[ 4240.578263] mem: Module loaded successfully (device number: 111, pool size: 4096)
[ 4242.695997] open(f252d300)
[ 4242.695999] mem: allocating 100 bytes
[ 4242.696253] mem: set idx 0
[ 4242.696280] mem: about to write Hello buddy
[ 4242.696285] mem: found string 11 long
[ 4242.696289] mem: could not write 0 bytes of 12 bytes
[ 4242.696314] mem: set idx 3
[ 4242.696338] mem: set read size 10
[ 4242.696363] mem: string 8 in mem lo buddy
[ 4242.696368] mem: could not read 0 bytes of 9
[ 4242.696494] mem: freeing idx 0
[ 4242.696502] release(f202ba00,f252d300)

