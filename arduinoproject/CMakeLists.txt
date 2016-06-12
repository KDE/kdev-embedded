#
# CMakeLists template to Arduino development.
#
#################################
# ARDUINO_PROTOCOL
# if upload is done by makefile
# protocol list http://www.nongnu.org/avrdude/user-manual/avrdude_4.html
#################################
# ARDUINO_VAR
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
# ARDUINO_PATH
# Arduino folder
# e.g: /usr/share/arduino or ~/.local/share/kdevelop/arduino-x.x.x
#################################

# Compile
set(ARDUINO_VAR          "mega")
set(ARDUINO_MCU          "atmega2560")
set(ARDUINO_FCPU         "16000000")

# Upload
set(ARDUINO_UPLOAD_SPEED "115200")
set(ARDUINO_PROTOCOL     "stk500v1")
set(ARDUINO_PORT         "/dev/ttyACM0")

# Arduino path
set(ARDUINO_PATH         "/usr/share/arduino")
#################################

project(%{APPNAMELC})

cmake_minimum_required(VERSION 2.8)

include(${CMAKE_SOURCE_DIR}/arduino.cmake)

set(MYPROJECT_SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${ARDUINO_SOURCE_FILES}
)

add_executable(%{APPNAMELC}.elf ${MYPROJECT_SOURCE_FILES})
