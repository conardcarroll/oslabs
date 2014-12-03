Do this:
  sudo insmod driver-07.ko
  sudo mknod /dev/char_dev c 100 0
  sudo chmod a+w /dev/char_dev
  dmesg
or this:
  run-07

Then do this:
  ioctl-07
  stop-07

See comments in the code 
