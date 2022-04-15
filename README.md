# VirtualT-DevTerm
VirtualT-DevTerm is a TRS-80 Model 100/102/200 emulator by [deuce](https://sourceforge.net/u/deuce/profile/) & [kpettit1](https://sourceforge.net/u/kpettit1/profile/) repurposed for the ClockworkPi DevTerm A04 & A06 by [sboger](https://github.com/sboger)

Original source comes from the now defunct project at: https://sourceforge.net/projects/virtualt/

Instructions for building on DevTerm A06 and A04:

* sudo apt install libfltk1.3-dev libjpeg-dev libpng-dev libxft-dev libxinerama-dev libxext-dev
* git clone https://github.com/sboger/VirtualT-DevTerm.git
* cd VirtualT-DevTerm 
* make clean
* make
* ./virtualt-devterm


Instructions to run pre-built binary (untested, built on A06):

* git clone https://github.com/sboger/VirtualT-DevTerm.git
* cd VirtualT-DevTerm
* chmod +x virtualt-devterm
* ./virtualt-devterm


Screenshots:

![Screenshot](/doc/Screenshot_2022-04-12_16-30-12.png)

![Screenshot](/doc/Screenshot%20from%202022-04-15%2002-06-26.png)

![Screenshot](/doc/devterm-basic.gif)


Todo:
* ~~Makefile cleanup to allow building on A04 (in progress)~~
* ~~Cruft cleanup. Going DevTerm specific.~~
* Make deb packages
* Fullscreen fix
