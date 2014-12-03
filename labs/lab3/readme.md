#Lab 3
I did this lab in two parts like Lab 2 so I'll try to upload it as such. I got alot of help early on from Conard on running the VM but he didn't have that much to do with the actual coding. I originally had this document in GDocs but have learned the awesomeness of Markdown since then. If you look at the Final??? output below, you'll see where I ended up spending way to much time on this lab attempting to recreate those results. How can the two threads each do exactly half of the work? Or even close to half?  Also please take the next paragraph as how committed I am to learning about the interesting topics you discuss in your class. :)


###Move to vagrant
With the help of Conard Carroll, an EET student, I was able to utilize vagrant instead of using the GUI virtualbox interface. This allowed me to quickly get over a number of hurdles. The biggest problem I had was I had installed the lab3_part2 module and was never able to unload it. I’m not sure I actually solved the problem but vagrant allowed me to quickly restart my VM and just SSH into it and only use the command line for make, insmod, dmesg and rmmod.  There was one gotcha which Conard quickly helped me overcome, that of assigning more than one core to the VM. The Vagrant documentation says assigning the number of CPUs using the VirtualBox provider is missing a step. This was found by looking at the VirtualBox GUI (not the actual desktop of the VM) and looking at the setting of the VM. The setting dialog said there were incorrect setting (yay!). The IO APIC had to be set ON in order for the VM to have more than one CPU assigned. Because Part 2 shouldn’t require schedule(), even though I did see what happens when it is used, this is why the output is provided. I’ve included the Vagrantfile I used in hopes you will look into this method for your own use and possibly suggesting it to your future students  (Hopefully I upload that file as well with this but if I forget I have appended it to the end of this document, forgive the lack of syntax color).

The best part of using vargrant with VirtualBox has been the automagic shared folder, allowing me to edit code files on my host machine and then just make/run using vagrant ssh on the guest VM.    


Part 1:

With schedule():

[  174.715357] Starting..
[  174.715795] Consumer TID 1
[  174.716386] Consumer TID 2
[  176.087170] Consumer 2 done
[  176.087174] Consumer 1 done
[  182.447501] stat 1 is 99672
[  182.447505] stat 2 is 328
[  182.447568] cs1 512482 cs2 492406 cs1+cs2 1004888
[  182.447571] t1 stop
[  182.447637] module unload

Without schedule():
[ 5385.540228] Starting..
[ 5385.540990] Consumer TID 1
[ 5385.542019] Consumer 1 done
[ 5385.542044] Consumer TID 2
[ 5385.542047] Consumer 2 done
[ 5395.579984] stat 1 is 100000
[ 5395.580120] cs1 1000000 cs2 0 cs1+cs2 1000000
[ 5395.580123] t1 stop
[ 5395.580139] module unload

With schedule, id declared in init
[ 1050.010410] Starting..
[ 1050.011006] Consumer TID 1
[ 1050.012229] Consumer TID 2
[ 1051.272190] Consumer 1 done
[ 1051.272194] Consumer 2 done
[ 1098.053168] stat 1 is 99657
[ 1098.053172] stat 2 is 343
[ 1098.053227] cs1 501268 cs2 502643 cs1+cs2 1003911
[ 1098.053244] module unload

With schedule, id declared globally
[ 1194.201480] Starting..
[ 1194.201685] Consumer TID 1
[ 1194.202411] Consumer TID 2
[ 1195.552156] Consumer 2 done
[ 1195.552161] Consumer 1 done
[ 1200.310535] stat 1 is 99421
[ 1200.310540] stat 2 is 579
[ 1200.310596] cs1 506248 cs2 503845 cs1+cs2 1010093
[ 1200.310656] module unload


Part 2:
one core:
[ 5465.568735] Starting..
[ 5465.569867] Consumer TID 1
[ 5465.614555] Consumer 1 done
[ 5465.615607] Consumer TID 2
[ 5465.615614] Consumer 2 done
[ 5481.070222] stat 1 is 100000
[ 5481.070279] cs1 1000000 cs2 0 cs1+cs2 1000000
[ 5481.070282] t1 stop
[ 5481.070340] module unload

w/o schedule and semaphore added
[  152.669818] t1 no stop
[  152.669895] t2 no stop
[  152.670317] stat 1 is 100000
[  152.670442] cs1 500260 cs2 499740 cs1+cs2 1000000
[  152.670447] module unload

with schedule and semaphore
[  905.140694] t1 no stop
[  905.140754] t2 no stop
[  905.141053] stat 1 is 100000
[  905.141112] cs1 499940 cs2 500060 cs1+cs2 1000000
[  905.141115] module unload

removing modules before completed
[ 1337.973066] Consumer 1 idx 999999
[ 1337.973071] Consumer 2 entered lock
[ 1337.973076] Consumer 2 upped
[ 1337.973081] Consumer 2 done
[ 1337.973103] Consumer 1 entered lock
[ 1337.973106] Consumer 1 broke out
[ 1337.973108] Consumer 1 done
[ 1337.973117] t1 stop
[ 1337.973186] t1 no stop
[ 1337.973239] t2 no stop
[ 1337.973499] stat 1 is 100000
[ 1337.973550] cs1 500001 cs2 499999 cs1+cs2 1000000
[ 1337.973553] module unload

Final???
[ 1450.861732] Consumer 2 idx 999999
[ 1450.861737] Consumer 1 entered lock
[ 1450.861743] Consumer 1 upped
[ 1450.861749] Consumer 1 done
[ 1450.861773] Consumer 2 entered lock
[ 1450.861776] Consumer 2 broke out
[ 1450.861779] Consumer 2 done
[ 1456.676155] t1 no stop
[ 1456.676230] t2 no stop
[ 1456.676538] stat 1 is 100000
[ 1456.676594] cs1 500000 cs2 500000 cs1+cs2 1000000
[ 1456.676597] module unload
Further runs were attempted only to recreate the same results, which shouldn’t be expected but only to see if there was something weird going on. 

W/o schedule
[ 1556.652883] Consumer 1 idx 999999
[ 1556.652888] Consumer 2 entered lock
[ 1556.652891] Consumer 2 upped
[ 1556.652896] Consumer 2 done
[ 1556.652919] Consumer 1 entered lock
[ 1556.652922] Consumer 1 broke out
[ 1556.652925] Consumer 1 done
[ 1561.708738] t1 no stop
[ 1561.708808] t2 no stop
[ 1561.709107] stat 1 is 100000
[ 1561.709162] cs1 500264 cs2 499736 cs1+cs2 1000000
[ 1561.709166] module unload

With schedule() again?
[ 1614.828776] Consumer 1 idx 999999
[ 1614.828781] Consumer 2 entered lock
[ 1614.828785] Consumer 2 upped
[ 1614.828790] Consumer 2 done
[ 1614.828813] Consumer 1 entered lock
[ 1614.828816] Consumer 1 broke out
[ 1614.828818] Consumer 1 done
[ 1620.833142] t1 no stop
[ 1620.833161] t2 no stop
[ 1620.833466] stat 1 is 100000
[ 1620.833525] cs1 500194 cs2 499806 cs1+cs2 1000000
[ 1620.833528] module unload

Stopping before done with schedule()
[ 1669.372811] Consumer 2 entered lock
[ 1669.372816] Consumer 2 upped
[ 1669.372822] Consumer 2 idx 999999
[ 1669.372827] Consumer 1 entered lock
[ 1669.372831] Consumer 1 upped
[ 1669.372836] Consumer 1 done
[ 1669.372896] Consumer 2 entered lock
[ 1669.372899] t1 stop
[ 1669.372903] t1 no stop
[ 1669.374361] Consumer 2 broke out
[ 1669.374531] Consumer 2 done
[ 1669.374591] t2 stop
[ 1669.374604] t2 no stop
[ 1669.374949] stat 1 is 100000
[ 1669.375130] cs1 500080 cs2 499920 cs1+cs2 1000000
[ 1669.375132] module unload



Vagrantfile:

```
# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|

  config.vm.box = "hashicorp/precise32"
  config.vm.graceful_halt_timeout = 15
  config.vm.provider "virtualbox" do |vb|
   vb.customize ["modifyvm", :id, "--ioapic", "on"]
   vb.customize ["modifyvm", :id, "--cpus", "2"]
   vb.customize ["modifyvm", :id, "--memory", "2048"]
  end
end
```