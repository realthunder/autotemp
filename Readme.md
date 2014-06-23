# Overview

This project is aimed to create an wireless networked automatic room temperature control system. The basic idea is to find some arduino like board with temperature and humidity sensor to sense room envrionment, and use IR remote control or other radio control board to control existing home appliance, such as electric fan, aircon. Multiple such devices can be linked either using wifi, which requires additional hardware, or use other wireless protocol, such as [Wixels](http://www.pololu.com/category/91/wireless) (this one is really promising, since it bundles with a microcontroller, so we don't need a separate arduino board)

# Change Log

## 2014.06.24

Added support for pro mini 328. 
Added patch to arduino core to support changing of PWM frequency without affecting millis()

## 2014.05.02

Added servo control function to shell.
```bash
# Servo control

# Swing the servo.
sv(1,<delay>) 
# The second argument controls the delay in milliseconds for each degree.Delay defaults to 15

# Position the servo
sv(2,<degree>)
```

## 2014.04.28

Increased temperature and humidity reading accuracy
Implemented aircon control script based on temperature/humidity monitoring.

## 2014.04.15

Added motor control function to shell.
```bash
# Motor control.
mt(<speed>,<on_time>,<off_time>)
# If no argument is specified, the motor is turned off.
# <Speed> ranges from 0~255. My PC USB can only seems to drive a baby safe fan to 100 max. 
#   The USB port easily gets stucked once beyond that value.
# <on_time> specifies the on period in seconds. If omitted, the motor is always on. 
# <off_time> specifies the off period in seconds. If omitted, the motor is off by half the 
#   <on_time> if given. The intension is to simulate natrual wind when drive a fan.
```

Added 5104 IR code sending capability. 
```bash
# Send 5104 IR code
ir(1,-2,<user_code>, <code>).
# For details of 5104 IR code, please check the document in doc directory. The reason for 
#   explicitly adding this code send instead of rely on raw code is because I got an bursted
#   remote controller using this code. So I have to dig into datasheet in order to recreate
#   the code sending.
```

## 2014.04.05

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

You will need [bitlash](http://bitlash.net) to compile the project. You need a patched version from [here](https://github.com/realthunder/bitlash).

To compile and upload, simply use the following command
```
make upload
```

To compile the project for [pro mini 328](https://www.sparkfun.com/products/11113), you'll need a patch in the patch directory to add support of changing arduino pwm frequency without affecting millis() function. The patch is against arduino 1.0.5. To apply the patch
```
cd <path-to-arduino>
patch -p1 < <path-to-patch>
```

You also need a set of arduino makefiles from [here](https://github.com/realthunder/Arduino-Makefile). Clone it to the same directory where you put the autotemp project.

To compile and upload,
```
make board=pro2 upload
```

