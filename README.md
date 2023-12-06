
neopixel christmas lights
  for raspberry pi pico

    --- by molen ---



building & running
------------------

assuming you:
* are using vscode
* have pico sdk & cmake installed
* have the cmake extension for vscode installed

then, when you open the folder in vscode, it should do something sensible to set up cmake and generate makefiles

`.\vcsetup.bat`
`cd build`
`nmake`

assuming you have the pico in bootloader mode, you can then run

`copy hohoho\hohoho.uf2 d:` _(change the drive letter to wherever your pico mounted itself)_


i chose pin 40(VBUS) for the V+ connection, 38(GND) for the ground connection, and pin 34(GP28) for the DATA/PIXEL line
