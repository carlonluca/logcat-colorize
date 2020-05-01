# logcat-colorize

A simple program that colorizes Android Debug Bridge (adb)'s logcat output on a terminal window.

*Notes*:

  - supports output formats: brief, tag, process, time or threadtime (see more about this in the official [docs][1]);
  - works on Linux and Mac OS;

![image][2]

# Installation

## Snap

You can install logcat-colorize from Snappy: https://snapcraft.io/logcat-colorize.

    sudo snap install logcat-colorize

## Macports

You can install logcat-colorize from macports: https://github.com/macports/macports-ports/blob/master/devel/logcat-colorize/Portfile

    sudo port install logcat-colorize

## DIY (from sources)

This depends on:

  * libboost-regex
  * libboost-program-options

If you are on Debian/Ubuntu:
    
        $ sudo apt-get install -y build-essential libboost-regex-dev libboost-program-options-dev

If you are on Mac OS X (using macports with libs installed in /opt/local):

        $ sudo port install boost

Compile and install:

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
        $ adb logcat -v time | logcat-colorize | egrep -i '(sensor|wifi)'

        # Save logcat output to file and read later with logcat colorize.
        $ adb logcat > /tmp/logcat.txt
        $ cat /tmp/logcat.txt | logcat-colorize
        
        # List available formats, then set a specific format for debug messages.
        # Set in your ~/.bash_profile to make it permanent.
        $ logcat-colorize --list-ansi
        $ export LOGCAT_COLORIZE_MSG_DEBUG="^[4;44;33m"
        $ adb logcat | logcat-colorize

**Note**: I had written this as a quick approach in bash, but turns out it is pretty slow, specially pulling logcat from new devices (really a lot). So I decided to go a bit lower level and re-wrote this in C++. For reference, if you want to see the bash version, check out tag 0.2.0 (3f1486234a).


[1]: http://developer.android.com/tools/debugging/debugging-log.html#outputFormat
[2]: https://github.com/carlonluca/logcat-colorize/blob/master/extras/shot.png
