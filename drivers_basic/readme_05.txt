Do this:
  sudo insmod driver-05.ko
  cat /proc/devices | grep arr_dev_05
  dmesg -> get the major number
  sudo mknod /dev/temp c 250 0 
  sudo chmod a+w /dev/arr_dev_05
or do this:
  run-05

Then do this:
  open 3 terminals - then quickly do this:
  (1) tryit write "Hello buddy"
  (2) tryit read
  (3) tryit read
  dmesg - to see what happened

  note that both "read"s went into the semaphore - took out what was written

Then do this:
  (1) tryit write "Hello Buddy"
  (2) tryit write "Hello There"

  note that one "write" enters the semaphore at a time

  stop-05

See comments in the code
