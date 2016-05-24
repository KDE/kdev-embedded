#################################
# ARDUINO_PROTOCOL
# if upload is done by makefile
# protocol list http://www.nongnu.org/avrdude/user-manual/avrdude_4.html
#################################
# ARDUINO_BOARD
# board variant
# types can be found here: arduino/hardware/arduino/avr/variants/*
#################################
# ARDUINO_MCU
# board microcontroller
# list: $ avr-gcc -mmcu=x
#################################
# ARDUINO_FCPU
# board clock in Hz
#################################
# ARDUINO_UPLOAD_SPEED
# upload baudrate
#################################
# ARDUINO_PORT
# usb interface to upload
# look at: $ lsusb or $ ls /dev/tty*
#################################

# compile
set(ARDUINO_BOARD     "mega")
set(ARDUINO_MCU       "atmega2560")
set(ARDUINO_FCPU      "16000000")

# upload
set(ARDUINO_UPLOAD_SPEED "115200")
set(ARDUINO_PROTOCOL     "stk500v1")
set(ARDUINO_PORT         "/dev/ttyACM0")

#################################

project(%{APPNAMELC})

cmake_minimum_required(VERSION 2.6)

include(${CMAKE_SOURCE_DIR}/arduino.cmake)

set(MYPROJECT_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${ARDUINO_SOURCE_FILES}
)

add_executable(%{APPNAMELC}.elf ${MYPROJECT_SOURCE_FILES})
