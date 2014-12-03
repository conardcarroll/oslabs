Do this:
  sudo insmod driver-02.ko
  MAJOR=`dmesg | tail -n 1 | awk "{print \\$5}"`
  if [ -z $MAJOR ]; then
     MAJOR=`dmesg | tail -n 1 | awk "{print \\$4}"`
  fi
  sudo mknod /dev/arr_dev_02 c $MAJOR 0
  sudo chmod a+w /dev/arr_dev_02
or this:
  run-02

Note: /dev/arr_dev_02 exists
Note: the major number may be different from 247. 
      there is a "arr_dev_03" entry in /proc/devices somewhere 
      around Major number 250

Then do this:
  cat /dev/arr_dev_02
  dmesg
  (result -> Inside open
             Inside read
             Inside close)
  echo "Hello my name is Harry" > /dev/arr_dev_02
  dmesg
  (result -> Inside open
             Inside write, buffer=Hello my name is Harry, size=23
             Inside close)
  stop-02

See comments in the code
