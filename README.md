# VirtualT-DevTerm
VirtualT is a TRS-80 Model 100/102/200 emulator by [deuce](https://sourceforge.net/u/deuce/profile/) & [kpettit1](https://sourceforge.net/u/kpettit1/profile/), repurposed for the ClockworkPi DevTerm by [sboger](https://github.com/sboger)

Original source comes from the now defunct project at: https://sourceforge.net/projects/virtualt/

Instructions for building on DevTerm-A06 and A04.

* git clone https://github.com/sboger/VirtualT-DevTerm.git
* cd VirtualT-DevTerm 
* sudo apt install libfltk1.3-dev libjpeg-dev libpng-dev libxft-dev libxinerama-dev libxext-dev
* make
* ./virtualt-devterm

![Screenshot](/doc/Screenshot_2022-04-12_16-30-12.png)



Todo:
* ~~Makefile cleanup to allow building on A04 (in progress)~~
* Cruft cleanup. Going DevTerm specific.
* Make deb packages
* Fullscreen fix
