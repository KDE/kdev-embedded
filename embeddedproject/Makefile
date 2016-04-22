# Command to read/write FLASH/EEPROM images. Currently
# Must have avrdude interface
# %{board_name} %{board_clock}
AVRDUDE = /usr/local/bin/avrdude

# Device options for AVRDUDE Command.
AVRDEVICE = -c usbasp -p m64

AVRCOMMAND = $(AVRDUDE) $(AVRDEVICE)

WRITE_COMMAND = $(AVRCOMMAND) -U flash:w:opentx.hex:i
