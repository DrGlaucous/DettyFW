# Detty Firmware
This is firmware intended to run on an ESP32 in order to control a brushless nerf blaster. 

This was originally part of the [G4](https://github.com/DrGlaucous/G4) repository, but was split off as the firmware became more complex. The G4 repo now mainly contains hardware and CAD for the blaster.


## Branch info

This tries to cut out bloat in the other programs and streamline operation and modularity.

I intend to use hardware inturrupts (because I've found that freeRTOS tasks are not fast enough) to run the program




<details>
<summary style="font-size:80%;"><i><b>Notes for me</b></i></summary>





</details>


### Configuration
All settings for the blaster are configured using the `settings.json` file stored on its internal filesytem.










