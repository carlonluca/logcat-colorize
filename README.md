# logcat-colorize

A simple script that colorizes Android Debug Bridge (adb)'s logcat output on 
a terminal window.

Some notes:
  - supports output formats as brief or time (see more about this in the official ![docs][1])
  - works on Linux (maybe on Mac as well)
  - a bit slow for too much logcat data, but once it catches up, it is ideal to use (consider also using filters)

# Installation

    # save the logcat-colorize to a directory of your PATH
    # (usually /usr/local/bin)
    # set execution permission to the file
    
    $ sudo chmod +x /usr/local/bin/logcat-colorize
    
    # you are ready!

# Usage


    # Simplest usage:
    $ logcat-colorize adb logcat 

    # Using specific device, with time details, and filtering:
    $ logcat-colorize adb -s emulator-5556 logcat -v time System.err:V *:S 

    # Piping to grep for regex filtering (much better than adb filter):
    $ logcat-colorize adb logcat -v time | egrep -i '(sensor|wifi)'


That's it!


[1]: http://developer.android.com/tools/debugging/debugging-log.html#outputFormat

