# logcat-colorize

A simple script that colorizes Android Debug Bridge (adb)'s logcat output on 
a terminal window.

*Update*: I had written this as a quick approach in bash, but turns out it is pretty slow, specially pulling logcat from new devices (really a lot). So I decided to go a bit lower level and re-wrote this in C++. 

Some notes:
  - supports output formats as brief or time (see more about this in the official [docs][1])
  - works on Linux

![image][2]

# Installation

    # download (or clone) the source
    $ make
    $ sudo make install

# Usage

    # Help and version info:
    $ logcat-colorize

    # Simplest usage:
    $ adb logcat | logcat-colorize

    # Using specific device, with time details, and filtering:
    $ adb -s emulator-5556 logcat -v time System.err:V *:S | logcat-colorize

    # Piping to grep for regex filtering (much better than adb filter):
    $ adb logcat -v time | egrep -i '(sensor|wifi)' | logcat-colorize


That's it!


[1]: http://developer.android.com/tools/debugging/debugging-log.html#outputFormat
[2]: https://bitbucket.org/brunobraga/logcat-colorize/downloads/example.jpg

