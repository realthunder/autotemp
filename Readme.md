# Overview
This project is aimed to create an wireless networked automatic room temperature control system. The basic idea is to find some arduino like board with temperature and humidity sensor to sense room envrionment, and use IR remote control or other radio control board to control existing home appliance, such as electric fan, aircon. Multiple such devices can be linked either using wifi, which requires additional hardware, or use other wireless protocol, such as [Wixels](http://www.pololu.com/category/91/wireless) (this one is really promising, since it bundles with a microcontroller, so we don't need a separate arduino board)

# Status

Currently implemented using [teensy3](http://www.pjrc.com/teensy/index.html) connected with some IR led from a busted electric fan remote controller. See [here](https://www.sparkfun.com/products/10732) for and example for connecting the led. It can now successfully control the fan. Since the remote is busted in the first place, so I can't use the normal record and playback procedure. Luckily, the decoder chip in the remote is easy to track down. It uses a code called 5104, and chip is from China (see the datasheet in doc directory. It's in chinese). It is supposedly a common protocol used in simple appliance like the electic fan.

# Compile

To compile the project, we'll need the follow the teensy website [instruction](http://www.pjrc.com/teensy/first_use.html) to install the teensyduino. I'm a heavy vim user, so I also borrowed makefile from [here](http://forum.pjrc.com/threads/23605-Teensy-mk-port-of-Arduino-mk-Makefile). I slightly modified the makefile to make it work with ``cygwin``, and is available [here](https://gist.github.com/realthunder/9374708). You need to modify it to set ``ARDUINO_DIR`` to your arduino installation direction. You probably need to install the arduino in a none spaced directory to make cygwin happy.

