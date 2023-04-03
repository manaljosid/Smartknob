# Smartknob
A smartknob clone based on the [original project](https://github.com/scottbez1/smartknob) from Scott Bezek.

A couple changes are implemented including:
- Change of microcontroller from ESP32 to RP2040
- Change of load cell setup from dedicated loadcells to resistors
- Change of HX711 load cell amplifier to MCP3564R 24-bit ADC
- Change of RGB LEDs to ones from Adafruit

The project uses Altium Designer for schematic and PCB design, Autodesk Fusion 360 for mechanical design and the Pico SDK for software development.

## Heads-up!
This repository is under heavy development. No guarantee is made that anything in it will work. Currently all changes are committed directly to the main branch and as such the main branch will mostly contain known bugs.

## TODO
- Add submodule fetching instructions
- Check that fetching submodules actually works
- Add mechanical design from Fusion 360
- Add installation instructins to the component library

## Installation
### Schematic and PCB design
The designs are made with the latest version of Altium Designer and as such the software along with a valid license is required.

In addition to Altium Designer the component library used for all of my projects needs to be installed. It can be found on this [github page](https://github.com/manaljosid/Moonlight_Library).

### Mechanical design
The mechanical design is not available at the moment other than the STL files used to 3D print. I plan to add it here in the future.

### Software development
In addition to pulling this repository you need to fetch all required submodules. Those include the actual pico-sdk and its submodules. Make sure the submodules are all installed, if the submodules of the pico-sdk are not installed then no USB function other than the boot ROM will work.

Due to bugs in the current release of the pico-sdk (mainly that the clock configuration is hard coded) a development branch is used.

## Contributing
To contribute to this project I ask you to either add an issue (make sure to check your issue does not exist before adding a new one) or make a fork of the repository and then issue a pull request with your changes.