# Firefly

# Software prerequisites
* Segger Embedded Studio for ARM (SES). 
  Tested with [v4.12](https://www.segger.com/downloads/embedded-studio/Setup_EmbeddedStudio_ARM_v412_win_x64.exe),
  all versions are available [here](https://www.segger.com/downloads/embedded-studio/).
  At some point after launch, it will request a license. Fortunately, it's free for nRF development - just fill all fields and request it.
* [nRF SDK 15.3.0](https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v15.x.x/nRF5_SDK_15.3.0_59ac345.zip).
  Download and unzip it anywhere (e.g. `C:/Dev/nRF5_SDK_15.3.0_59ac345`), then in SES go to Tools -> Options -> Building -> Global macros
  and set NRF_SDK_PATH to path chosen above (e.g. `NRF_SDK_PATH=C:/Dev/nRF5_SDK_15.3.0_59ac345`).
  
That should be enough. If needed, more information and links to the additional tools can be in the official
Nordic's ["Getting Started"](https://infocenter.nordicsemi.com/topic/ug_gsg_ses/UG/gsg/install_toolchain.html?cp=1_1_0_6).

# Hardware setup
Requirements:
* One of [nRF devkits](https://www.nordicsemi.com/Software-and-Tools/Development-Kits).
* Some jumper wires.
* Firefly board itself.

Connect `VDD`, `GND`, `SWDIO` and `SWDCLK` according to the 
[devkit](docs/devkit_programmer_pinout.png) and [Firefly](docs/firefly_programmer_pinout.jpg) pinouts.
