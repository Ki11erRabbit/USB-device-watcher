# USB-device-watcher
Can't figure out Udev? This allows you to run a command when a usb device is plugged or unplugged

This not very well named peice of software monitors USB devices on Linux systems and allows users to execute commands when a specific USB device is plugged in or unplugged. This program should not be run as root and currently terminates if run as root.
## How it works
- It begins by parsing a configuration file (via a function called `setupDevices`) at `~/.config/` (`/home/<user's home>/.config`) called `USB-watcher.conf`. The format is specified below.
- It works by parsing `/sys/bus/usb/devices/` and only searching the files that contain a `-`(hyphen) which will be a currently plugged in USB device.
-Then the code will parse in those directories to find idVendor and idProduct which contain the necessary information to identify a devices. If the ids match then a flag will get marked indicating that a particular devices has been found. This is done in the `findDevices` function.
- After going through all directories in `/sys/bus/usb/devices/` then the the code will check if the found flag was set and execute the command specified and set another flag called activated. This is to prevent the command from running constantly and to know if the unplug command should be run. This is done via the `executeDevices` function.
- Then if a device was unplugged and it had a command run, then the unplug command will be executed.
- `Sleep(1)` is called at the end of the loop to slow down execution to prevent the code from running too fast.

## Configuration File Specifications
The format is as follows 
```
vendorID/productID
CMD to be executed when found
CMD to be executed when no longer plugged in
```
Comments can included by begining a line with a `#` (hash symbol, pound sign, etc). Comments cannot be on the same line as the ids or commands.
Newlines are ignored but can be included for better readability. By default the device limit is 50 devices but that can be changed by editing `MAXDEVICES`.

### Example Configuration File
```
# this is a comment
17f6/0822
model-m-helper
setxkbmap -layout us
```
