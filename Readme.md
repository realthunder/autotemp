# Overview
This project is aimed to create an wireless networked automatic room temperature control system. The basic idea is to find some arduino like board with temperature and humidity sensor to sense room envrionment, and use IR remote control or other radio control board to control existing home appliance, such as electric fan, aircon. Multiple such devices can be linked either using wifi, which requires additional hardware, or use other wireless protocol, such as [Wixels](http://www.pololu.com/category/91/wireless) (this one is really promising, since it bundles with a microcontroller, so we don't need a separate arduino board)

# Change Log

## 2014.05.04

Added [Bitlash](http://bitlash.net) for simple shell. Added the following user functions,
```bash
# Temperature and humidity reading.
# Optional argument specifies the I2C polling interval in milliseconds.
# Returns the last read value.
th(<polling_interval>)

# Infra Red signal receiving and transmitting
#
# General form
ir(<cmd>,...)
# If <cmd> is omitted, it will transmits the last received IR code.
#
# IR receiving
ir(0,<wait_period>)
# Optional argument specifies the wait period in milliseconds, for which the device and wait
# for any IR signal before timeout.
# The return text output is and exact command sequence that can reproduce the received signal
#
# IR transmitting
ir(1,[code_type],<code_length>, <code_value>)
# If [code_type] is anything other than -1 (i.e. raw data), you need to specify the code length and
# value. Refer to arduino IRRemote library for code type. or just copy the output of ir(0). 
# For raw data sending, you can prepare the data using  the following raw buffer setup commands.
#
# IR raw buffer setup
ir(2,[value]...)
ir(3,[value]...)
# ir(2,...) resets the internal raw buffer index to 0, and squentially fill the buffer with
# the input argument. ir(3,...) continues the filling. This function exists because of the 
# limited line buffer size of bitlash, which is in turn limited by the arduino device memory.
```

## 2014.03.20

Initial prototype is built using [teensy3](http://www.pjrc.com/teensy/index.html) connected with some IR led from a busted electric fan remote controller. See [here](https://www.sparkfun.com/products/10732) for an example for connecting the led. It can now successfully control the fan. Since the remote is busted in the first place, so I can't use the normal record and playback procedure. Luckily, the encoder chip in the remote is easy to track down. It uses a code called 5104, and chip is from China (see the datasheet in doc directory. It's in chinese). It is supposedly a common protocol used in simple appliance like the electic fan.

# Compile

To compile the project, we'll need the follow the teensy website [instruction](http://www.pjrc.com/teensy/first_use.html) to install the teensyduino. I'm a heavy vim user, so I also borrowed makefile from [here](http://forum.pjrc.com/threads/23605-Teensy-mk-port-of-Arduino-mk-Makefile). I slightly modified the makefile to make it work with ``cygwin``, and is available [here](https://gist.github.com/realthunder/9374708). You need to modify it to set ``ARDUINO_DIR`` to your arduino installation direction. You probably need to install the arduino in a none spaced directory to make cygwin happy.

You will need [bitlash](http://bitlash.net) to compile the project. You can either obtain the official release, or from my fork [here](https://github.com/realthunder/bitlash). Although they are the same at the moment, I may need to personalize it in the future.

