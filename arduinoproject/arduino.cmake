set(CMAKE_SYSTEM_NAME Generic)

enable_language(C)
enable_language(ASM)

cmake_policy(SET CMP0046 OLD)

#Arduino path
set(ARDUINO_CORE_DIR "${ARDUINO_PATH}/hardware/arduino/avr/cores/arduino/")
set(ARDUINO_PINS_DIR "${ARDUINO_PATH}/hardware/arduino/avr/variants/${ARDUINO_BOARD}")
set(ARDUINO_BIN_DIR  "${ARDUINO_PATH}/hardware/tools/avr/bin/")

set(CMAKE_ASM_COMPILER  "${ARDUINO_BIN_DIR}/avr-gcc")
set(CMAKE_AR            "${ARDUINO_BIN_DIR}/avr-ar")
set(CMAKE_C_COMPILER    "${ARDUINO_BIN_DIR}/avr-gcc")
set(CMAKE_CXX_COMPILER  "${ARDUINO_BIN_DIR}/avr-g++")
set(CMAKE_OBJCOPY       "${ARDUINO_BIN_DIR}/avr-objcopy")

set(AVROBJCOPY "${ARDUINO_BIN_DIR}/avr-objcopy")
set(AVRDUDE "${ARDUINO_BIN_DIR}/avrdude")

set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

# C only fine tunning
set(TUNNING_FLAGS "-funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums")

set(CMAKE_CXX_FLAGS "-w -Os -Wl,--gc-sections -mmcu=${ARDUINO_MCU} -DF_CPU=${ARDUINO_FCPU} -Os -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} ${TUNNING_FLAGS} -Wstrict-prototypes -w")

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

# FIXME: Forcing target name to be "firmware"
if(AVROBJCOPY AND AVRDUDE)
    add_custom_target(hex)
    add_dependencies(hex %{APPNAMELC})

    add_custom_command(TARGET hex POST_BUILD
        COMMAND ${AVROBJCOPY} -O ihex -R .eeprom ${CMAKE_CURRENT_BINARY_DIR}/%{APPNAMELC}.elf %{APPNAMELC}.hex
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
