Compile: make
Load:    sudo insmod brd.ko
Check:   ls /dev/ram* -> lists several deviced
         cat /proc/devices -> 1 ramdisk
         cd /sys/devices/virtual/block
         ls -> ram0 ... ram15 as directories
         cd ram0 -> look at some files via cat
         cat queue/logical_block_size -> 512
Format:  sudo mkfs.ext4 /dev/ram0
Mount:   sudo mkdir /f
         sudo mount /dev/ram0 /f
         df -> /dev/ram0  ext4  14839  140  13553  2% /f
         ls /f -> lost_found/
Adjust:  sudo rmdir /f/lost+found
	 sudo chmod a+w /f
         ls -al /f ->
            total 5
            drwxrwxrwx   2 root root 1024 Oct 10 13:27 ./
            dr-xr-xr-x. 29 root root 4096 Oct 10 09:24 ../
         df -> /dev/ram0  ext4  14839  128  13565  2% /f
Write:   emacs -nw /f/hello_world.txt
         enter-> Hello World -> save -> quit
         cat /f/hello_world.txt -> Hello World
         df -> /dev/ram0  ext4  14839  129  13564  1% /f
Read:    sudo ./tryit read -> Device data: <nothing>
Write:   sudo ./tryit write "Four score and seven years ago"
         df -> /dev/ram0  ext4  14839  129  13564  1% /f
Read:    sudo ./tryit read -> Device data: Four score and seven years ago
         cat /dev/ram0 ->
            Four score and seven years ago@3k9õ çâVRçâVRÿÿSïÇâVR...
            ! " # $ % & ' ( ) * + , - . / 0 1 2 3 4 5 6 7...
            .#hello_world.txt  Ìhello_world.txt... .ô..ÿ
            ÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿÿ...
            "äVRôfranco@franco.cinci.rr.com.14764:138141...
            !"#$%&'()*+,-./0123456789:;<=>?@A^[[?1;2c
            Note: "Hello World" is missing!!!
            cat /f/hello_world.txt -> Hello World
Unmount:  sudo umount /f
Unload:   sudo rmmod brd
