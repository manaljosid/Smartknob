# Smartknob
A smartknob clone based on the [original project](https://github.com/scottbez1/smartknob) from Scott Bezek.

A couple changes are implemented including:
- Change of microcontroller from ESP32 to RP2040
- Change of load cell setup from dedicated loadcells to resistors
- Change of HX711 load cell amplifier to MCP3564R 24-bit ADC
- Change of RGB LEDs to ones from Adafruit

The project uses Altium Designer for schematic and PCB design, Autodesk Fusion 360 for mechanical design and the Pico SDK for software development.

## Heads-up!
No guarantee is made that anything in this repository will work. Currently all changes are committed directly to the main branch and as such the main branch can contain bugs or have changes that have yet to be completed.

## Installation
### Schematic and PCB design
The designs are made with the latest version of Altium Designer and as such the software along with a valid license is required.

In addition to Altium Designer the component library used for all of my projects needs to be installed. It can be found on this [github repository](https://github.com/manaljosid/Moonlight_Library).

### Mechanical design
The mechanical design is not available at the moment other than the STL files used for 3D printing.

### Software development
In addition to pulling this repository you need to fetch all required submodules. Those include the actual pico-sdk and its submodules. Make sure the submodules are all installed, if the submodules of the pico-sdk are not installed then no USB function other than the boot ROM will work.

Due to issues in the release of the pico-sdk being used for this code (mainly that the clock configuration is hard coded) a development branch is used.

## Things to watch out for
The MCP3564R *NEEDS* a pull-up on the IRQ line when it is in high-z mode. It can be weak - about 100 kOhm will do but if it is not there the ADCDATA register will never contain any data. In this case it is solved using a pull-up on the RP2040s GPIO pin that's connected to the IRQ line.
