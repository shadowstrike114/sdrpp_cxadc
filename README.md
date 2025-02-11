# CXADC Card Source for SDR++

Using the alternative [driver for linux](https://github.com/happycube/cxadc-linux3), cheap Conexant C2388x based capture cards can be used as an 8-bit 28 MSPS ADC.
By doing some hardware mods as shown on [CXADC Wiki](https://github.com/happycube/cxadc-linux3/wiki/Modifications) and the [VHS Decode Project](https://github.com/oyvindln/vhs-decode/wiki/CX-Cards), they can sample at around 40 MSPS reliably.

This Plugin was originally written to take advantage of SDR++'s performance to visualize the spectrum of vhs signals in real-time to test the setup.

However, the cards can also be used to receive a large part of the HF spectrum when paired with a suitable antenna and amplifier.

![image](https://github.com/user-attachments/assets/6ca399e2-4615-4458-8e10-63d6613864c8)


Installation
--

The Plugin needs the [cxadc-linux3 driver](https://github.com/happycube/cxadc-linux3) driver to access the capture cards and is thus only available under linux.
To install, download `cxadc.so` from the Relases section and place it in your `/usr/lib/sdrpp/plugins` folder.

The Plugin can then be enabled in the SDR++ module manager.

To be able to control the ADC settings, your current user has to have the correct permissions to be able to modify the files under /sys/class/cxadc/cxadc<X>/device/parameters/.
Since the folder sometimes doesn't have the correct permissions even after installing the driver and joining the video group, it may be necessary to do the following:

Check if the permissions are correct (replace cxadc0 accordingly if you have more than one card)
```
echo 1 >/sys/class/cxadc/cxadc0/device/parameters/vmux
```
If this results in a Permission denied error, run 
```
sudo chown -R root:video /sys/class/cxadc/cxadc0/device/parameters/
```
to modify the permissions. This is not ideal and only lasts until reboot, but is the easiest method i found so far.

Building from Source
--

- Download the [SDR++](https://github.com/AlexandreRouma/SDRPlusPlus) Source code
- Open the top `level CMakeLists.txt` and add 
```
option(OPT_BUILD_CXADC_SOURCE "CXADC source" ON)
```
to the `Sources` section and 
```
if (OPT_BUILD_CXADC_SOURCE)
add_subdirectory("source_modules/cxadc_source")
endif (OPT_BUILD_CXADC_SOURCE)
```
to the `Source modules` section.

- clone or copy this repository into a folder called `source_modules/cxadc_source`   
- Build using the build guide from the SRDR++ repository
