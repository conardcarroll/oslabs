Do this:
  sudo insmod driver-04.ko
  cat /proc/devices | grep char
  (result -> 247 arr_dev_04)
  dmesg
  (result -> init: starting driver
             The major number for your device is 247)
  sudo mknod /dev/arr_dev_04 c 247 0
  sudo chmod a+w /dev/arr_dev_04
or do this:
  run-04

Then do this:
  echo "Hello Buddy" > /dev/arr_dev_04
  tryit-04 read
  cat /dev/arr_dev_04
  stop-04

See comments in the code


