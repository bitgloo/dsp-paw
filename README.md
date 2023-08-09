# dsp-paw

This is the DSP PAW "monorepo", containing the microcontroller firmware, add-on board design files, and the source code for the computer graphical interface (GUI).

*DSP PAW* (Digital Signal Processing Portable All-in-one Workstation) provides a solution for creating and studying DSP algorithms on embedded systems. Using the project's software and hardware with certain [NUCLEO development boards](https://www.st.com/en/evaluation-tools/stm32-nucleo-boards.html) will enable you to portably design DSP algorithms, without the need for external tools or laboratory equipment.

## Project components

1. [Firmware](https://code.bitgloo.com/bitgloo/dsp-paw/src/branch/main/firmware) that allows users to load and execute custom DSP algorithms for real-time signal processing.
2. [A custom add-on board](https://code.bitgloo.com/bitgloo/dsp-paw/src/branch/main/hardware) which provides the necessary circuitry for interfacing with external signals and the host computer.
3. [Computer software](https://code.bitgloo.com/bitgloo/dsp-paw/src/branch/main/gui) that facilitates algorithm design, execution, and detailed analysis.

## Features

* Real-time signal processing: signal read by the ADC are streamed through the loaded algorithm and output via the DAC.
* The core firmware facilitates algorithm uploads over USB, enabling a fast design and test process.
* Supports signal sampling rates from 8kS/s up to 96kS/s, with buffer size of up to 4,096 samples.
* Supports external signals between -2V and +2V.
* Two potentiometers for adjusting algorithm parameters while the algorithm is running.
* An on-board signal generator that eliminates the need for input signals from external hardware.
* Numerous analysis features, including signal visualization and algorithm execution time measurement, eliminate the need for other test equipment like oscilloscopes.

## Build notes

The add-on board was designed using [KiCad](https://www.kicad.org/).

Both the firmware and user interface software are written in C++, and have single Makefiles for their build processes. Both also depend on git submodules, so be sure to fetch those before compilation.

The firmware additionally requires the `arm-none-eabi` toolchain, and the GUI requires [SDL 2.0](https://www.libsdl.org/).

## Learn more

The [project's wiki](https://code.bitgloo.com/bitgloo/dsp-paw/wiki) will be updated over time with more information about all aspects of the project.

### Licensing

DSP PAW source code is licensed under version three of the GNU General Public License. The add-on board design files are licensed under the CERN Open Hardware License Version 2 - Strongly Reciprocal. See the `LICENSE` files in each component's folder for more information.

