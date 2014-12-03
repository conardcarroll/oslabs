Do this:
  sudo insmod driver-3.ko
  cat /proc/devices | grep char
  (result -> 247 arr_dev_03)
  dmesg
  (result -> init: starting driver
             The major number for your device is 247)
  sudo mknod /dev/arr_dev_03 c 247 0
or do this:
  Run run-03

Note: /dev/arr_dev_03 exists
Note: the major number may be different from 247. 
      there is a "arr_dev_03" entry in /proc/devices somewhere 
      around Major number 250

Then do this:
  tryit-03 write "Hello my name is Harry"
  dmesg:
  (result -> open: opening device
             release: closing device)
  tryit-03 read
  (result -> Device data: Hello my name is Harry...)
  dmesg
  (result -> open: opening device
             read: user data to device, cnt=100 sz=22
             release: closing device)
  cat /dev/arr_dev_03
  (result -> Hello my name is Harry)
  stop-03

See comments in the code
