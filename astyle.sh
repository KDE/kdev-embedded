astyle --style=allman -R "${PWD}/*.h" "${PWD}/*.cpp" "${PWD}/*.h.in" --unpad-paren / -U  --exclude="${PWD}/embeddedproject" --exclude="${PWD}/arduinoproject"
astyle --style=allman -R "${PWD}/*.h" "${PWD}/*.cpp" "${PWD}/*.h.in" --pad-header / -H --exclude="${PWD}/embeddedproject" --exclude="${PWD}/arduinoproject"

