# Lab 7 Network

## Remarks
This lab was fun to complete because it was mostly just following the instructions from the lab document and
making sure all the surrounding pieces were correct. I misspelled peanut about 3 times trying to get /etc/networks correct,
after I tried to open /etc/network with vi and wondering why my changes were never saved. I did use the Max Thrun code to check I had 
the everything I needed, but my problems were in incorrectly writing /etc files as I said before. There may be a few lines I added 
that he had but I don't think they had any impact on the results of the code. All the results listed below are exactly as expected
from the lab document. 

# Results

## First Test

$sudo tcpdump -i os0
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on os0, link-type EN10MB (Ethernet), capture size 65535 bytes
03:51:53.362024 40:00:40:01:b9:55 (oui Unknown) > 45:00:00:54:00:00 (oui Unknown), ethertype Unknown (0xc0a8), length 84:
	0x0000:  0001 c0a8 0002 0800 1eb2 0638 0001 5937  ...........8..Y7
	0x0010:  7d54 0c86 0500 0809 0a0b 0c0d 0e0f 1011  }T..............
	0x0020:  1213 1415 1617 1819 1a1b 1c1d 1e1f 2021  ...............!
	0x0030:  2223 2425 2627 2829 2a2b 2c2d 2e2f 3031  "#$%&'()*+,-./01
	0x0040:  3233 3435 3637

$ping 192.168.0.2
PING 192.168.0.2 (192.168.0.2) 56(84) bytes of data.
^C


## Final Driver

####Install
$ sudo insmod ./netdriver.ko

$dmesg
...
[   76.738141] loopback: os0 and os1 registered
[   76.738144] loopback: Module loaded successfully

####ifconfig
$ifconfig
os0       Link encap:Ethernet  HWaddr 00:01:02:03:04:05
          inet addr:192.168.0.1  Bcast:192.168.0.255  Mask:255.255.255.0
          inet6 addr: fe80::201:2ff:fe03:405/64 Scope:Link
          UP BROADCAST RUNNING NOARP MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)

os1       Link encap:Ethernet  HWaddr 00:00:00:00:00:01
          inet addr:192.168.1.1  Bcast:192.168.1.255  Mask:255.255.255.0
          inet6 addr: fe80::200:ff:fe00:1/64 Scope:Link
          UP BROADCAST RUNNING NOARP MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:0 errors:0 dropped:0 overruns:0 carrier:0
          collisions:0 txqueuelen:1000
          RX bytes:0 (0.0 B)  TX bytes:0 (0.0 B)
		  
####route
$ route
Kernel IP routing table
Destination     Gateway         Genmask         Flags Metric Ref    Use Iface
default         10.0.2.2        0.0.0.0         UG    0      0        0 eth0
default         10.0.2.2        0.0.0.0         UG    100    0        0 eth0
10.0.2.0        *               255.255.255.0   U     0      0        0 eth0
peanut          *               255.255.255.0   U     0      0        0 os0
grape           *               255.255.255.0   U     0      0        0 os1

####ping
$ ping 192.168.0.1
PING 192.168.0.1 (192.168.0.1) 56(84) bytes of data.
64 bytes from 192.168.0.1: icmp_req=1 ttl=64 time=0.258 ms
64 bytes from 192.168.0.1: icmp_req=2 ttl=64 time=0.042 ms
64 bytes from 192.168.0.1: icmp_req=3 ttl=64 time=0.043 ms
64 bytes from 192.168.0.1: icmp_req=4 ttl=64 time=0.036 ms
^C
--- 192.168.0.1 ping statistics ---
4 packets transmitted, 4 received, 0% packet loss, time 3044ms
rtt min/avg/max/mdev = 0.036/0.094/0.258/0.095 ms

####ssh
vagrant@precise32:/vagrant/labs/lab7$ ssh 192.168.1.1
vagrant@192.168.1.1's password:
Welcome to Ubuntu 12.04.5 LTS (GNU/Linux 3.2.0-23-generic-pae i686)

 * Documentation:  https://help.ubuntu.com/
New release '14.04.1 LTS' available.
Run 'do-release-upgrade' to upgrade to it.

Welcome to your Vagrant-built virtual machine.
Last login: Tue Dec  2 03:34:36 2014 from 10.0.2.2
vagrant@precise32:~$

####ping with tcpdump

$ ping butter-far
PING butter-far (192.168.0.2) 56(84) bytes of data.

$ sudo tcpdump -i os0
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on os0, link-type EN10MB (Ethernet), capture size 65535 bytes
03:47:44.436449 IP butter-near > butter-far: ICMP echo request, id 1543, seq 1, length 64
03:47:45.445343 IP butter-near > butter-far: ICMP echo request, id 1543, seq 2, length 64
03:47:46.445371 IP butter-near > butter-far: ICMP echo request, id 1543, seq 3, length 64


