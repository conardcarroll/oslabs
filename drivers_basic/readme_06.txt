Do this:
  sudo insmod driver-06.ko
  sudo mknod /dev/parlelport c 61 0
  sudo chmod a+w /dev/parlelport
  dmesg
or this:
  run-06

Then do this:
  cat /proc/ioports | grep 378       <- check to see if it is there
  flash-06
  ^C
  dmesg

  stop-06

See comments in the code
