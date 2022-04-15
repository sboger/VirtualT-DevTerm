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



Nostalgia (from https://en.wikipedia.org/wiki/TRS-80_Model_100):
* The TRS-80 Model 100 was a portable computer introduced in 1983. It is one of the first notebook-style computers, featuring a keyboard and liquid crystal display, in a battery-powered package roughly the size and shape of a notepad or large book.
* When first switched on, the Model 100 displays a menu of applications and files and the date and time. The ROM firmware-based system boots instantly
* The ROM also contains a terminal program, TELCOM; an address/phone book organizer, ADDRSS; a to-do list organizer, SCHEDL; and a simple text editor, TEXT. The TELCOM program allows automation of a login sequence to a remote system under control of the BASIC interpreter.
* The Model 100 was promoted as being able to run up to 20 hours and maintain memory up to 30 days on a set of four alkaline AA batteries.
* When introduced, the portability and simplicity of the Model 100 made it attractive to journalists, who could type about 11 pages of text and then transmit it for electronic editing and production using the built-in modem and TELCOM program.
* The Model 100 firmware was the last Microsoft product that Bill Gates developed personally, along with Suzuki. According to Gates, "part of my nostalgia about this machine is this was the last machine where I wrote a very high percentage of the code in the product"
