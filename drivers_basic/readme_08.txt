Do this:
  sudo insmod driver-08.ko
  dmesg
     The major number for your device is 247
  sudo mknod /dev/tester1 c 247 0
  sudo chmod a+w /dev/tester1
or this:
  run-08

Then do this:
  tryit-08 read &
  tryit-08 read &
  tryit-08 read &

  tryit-08 write "Hello" &
  tryit-08 write "Hello" &
  tryit-08 write "Hello" &

  stop-08

See comments in the code
Note: major number may be different from 247
