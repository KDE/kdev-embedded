# This file is based on the work of:
#
# http://mjo.tc/atelier/2009/02/arduino-cli.html
# http://johanneshoff.com/arduino-command-line.html
# http://www.arduino.cc/playground/Code/CmakeBuild
# http://www.tmpsantos.com.br/en/2010/12/arduino-uno-ubuntu-cmake/
# The libarduino-1.0 version is based on the work of:
# http://playground.arduino.cc/Code/Kdevelop


set(CMAKE_SYSTEM_NAME Generic)
enable_language(C)
enable_language(ASM)

set(CMAKE_ASM_COMPILER  /usr/bin/avr-gcc)
set(CMAKE_AR            /usr/bin/avr-ar)
set(CMAKE_C_COMPILER    /usr/bin/avr-gcc)
set(CMAKE_CXX_COMPILER  /usr/bin/avr-g++)
set(CMAKE_OBJCOPY       /usr/bin/avr-objcopy)
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

# C only fine tunning
set(TUNNING_FLAGS "-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums")

set(CMAKE_CXX_FLAGS "-w -Os -Wl,--gc-sections -mmcu=${ARDUINO_MCU} -DF_CPU=${ARDUINO_FCPU} -Os -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} ${TUNNING_FLAGS} -Wstrict-prototypes -w")

set(ARDUINO_CORE_DIR "/usr/share/arduino/hardware/arduino/avr/cores/arduino/")
set(ARDUINO_PINS_DIR "/usr/share/arduino/hardware/arduino/avr/variants/${ARDUINO_BOARD}")
include_directories(${ARDUINO_PINS_DIR})
include_directories(${ARDUINO_CORE_DIR})

set(ARDUINO_SOURCE_FILES
    # core
    #${ARDUINO_CORE_DIR}/wiring_pulse.S

    ${ARDUINO_CORE_DIR}/hooks.c
    ${ARDUINO_CORE_DIR}/WInterrupts.c
    ${ARDUINO_CORE_DIR}/wiring_analog.c
    ${ARDUINO_CORE_DIR}/wiring.c
    ${ARDUINO_CORE_DIR}/wiring_digital.c
    #${ARDUINO_CORE_DIR}/wiring_pulse.c

    ${ARDUINO_CORE_DIR}/abi.cpp
    ${ARDUINO_CORE_DIR}/CDC.cpp
    ${ARDUINO_CORE_DIR}/HardwareSerial0.cpp
    ${ARDUINO_CORE_DIR}/HardwareSerial1.cpp
    ${ARDUINO_CORE_DIR}/HardwareSerial2.cpp
    ${ARDUINO_CORE_DIR}/HardwareSerial3.cpp
    ${ARDUINO_CORE_DIR}/HardwareSerial.cpp
    ${ARDUINO_CORE_DIR}/IPAddress.cpp
    ${ARDUINO_CORE_DIR}/main.cpp
    ${ARDUINO_CORE_DIR}/new.cpp
    ${ARDUINO_CORE_DIR}/PluggableUSB.cpp
    ${ARDUINO_CORE_DIR}/Print.cpp
    ${ARDUINO_CORE_DIR}/Stream.cpp
    ${ARDUINO_CORE_DIR}/Tone.cpp
    ${ARDUINO_CORE_DIR}/USBCore.cpp
    ${ARDUINO_CORE_DIR}/WMath.cpp
    ${ARDUINO_CORE_DIR}/WString.cpp
)

set(PORT $ENV{ARDUINO_PORT})
if (NOT PORT)
    set(PORT ${ARDUINO_PORT})
endif()

set(AVROBJCOPY "/usr/bin/avr-objcopy")
set(AVRDUDE "/usr/bin/avrdude")

# FIXME: Forcing target name to be "firmware"
if(AVROBJCOPY AND AVRDUDE)
    add_custom_target(hex)
    add_dependencies(hex %{APPNAMELC})

    add_custom_command(TARGET hex POST_BUILD
        COMMAND ${AVROBJCOPY} -O ihex -R .eeprom ${CMAKE_CURRENT_BINARY_DIR}/%{APPNAMELC} %{APPNAMELC}.hex
    )

    add_custom_target(flash)
    add_dependencies(flash hex)

    add_custom_command(TARGET flash POST_BUILD
        COMMAND ${AVRDUDE} -P ${PORT} -b ${ARDUINO_UPLOAD_SPEED} -c ${ARDUINO_PROTOCOL} -p ${ARDUINO_MCU} -V -F -U flash:w:%{APPNAMELC}.hex:i
    )
endif()

add_custom_target(reset)
add_custom_command(TARGET reset POST_BUILD
    COMMAND echo 0 > ${PORT}
)
