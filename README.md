# VirtualT-DevTerm
VirtualT-DevTerm is a TRS-80 Model 100/102/200 emulator by [deuce](https://sourceforge.net/u/deuce/profile/) & [kpettit1](https://sourceforge.net/u/kpettit1/profile/) repurposed for the ClockworkPi DevTerm A04 & A06 by [sboger](https://github.com/sboger)

Original source comes from the now defunct project at: https://sourceforge.net/projects/virtualt/

**New: v1.7.3 5/23/2022 - Fullscreen mode! Thanks Yatli!

Instructions to run pre-built binary (Runs on A06. A04 untested.):

* sudo apt install libfltk-images1.3
* git clone https://github.com/sboger/VirtualT-DevTerm.git
* cd VirtualT-DevTerm
* ./virtualt-devterm

Instructions for building on DevTerm A04 and A06 (Recommended):

* sudo apt install libfltk1.3-dev libjpeg-dev libpng-dev libxft-dev libxinerama-dev libxext-dev
* git clone https://github.com/sboger/VirtualT-DevTerm.git
* cd VirtualT-DevTerm 
* make clean
* make
* ./virtualt-devterm

Notes:
* VirtualT-DevTerm starts in 3x display size by default. Click the Emulation menu and then the Display menu to choose 1x, 2x, 4x, or FULLSCREEN. Fullscreen is quirky. It might show the background or windows at the very bottom of the screen. It also doesn't restart into fullscreen mode properly - you'll need to click "fullscreen" again even though the button is selected.


Screenshots:

![Screenshot](/doc/devterm-fullscreen2.png)

![Screenshot](/doc/Screenshot%20from%202022-04-15%2002-06-26.png)

![Screenshot](/doc/devterm-basic.gif)

Complete TRS-80 Model 100 user manual: https://github.com/sboger/VirtualT-DevTerm/blob/master/trs80-m100-user-manual.pdf


Todo:
* ~~Makefile cleanup to allow building on A04 (in progress)~~
* ~~Cruft cleanup. Going DevTerm specific.~~
* Make deb packages
* Full-screen/select button cleanup
* ~~Fullscreen fix~~


Nostalgia (from https://en.wikipedia.org/wiki/TRS-80_Model_100):
* The TRS-80 Model 100 was a portable computer introduced in 1983. It is one of the first notebook-style computers, featuring a keyboard and liquid crystal display, in a battery-powered package roughly the size and shape of a notepad or large book.
* When first switched on, the Model 100 displays a menu of applications and files and the date and time. The ROM firmware-based system boots instantly
* The ROM also contains a terminal program, TELCOM; an address/phone book organizer, ADDRSS; a to-do list organizer, SCHEDL; and a simple text editor, TEXT. The TELCOM program allows automation of a login sequence to a remote system under control of the BASIC interpreter.
* The Model 100 was promoted as being able to run up to 20 hours and maintain memory up to 30 days on a set of four alkaline AA batteries.
* When introduced, the portability and simplicity of the Model 100 made it attractive to journalists, who could type about 11 pages of text and then transmit it for electronic editing and production using the built-in modem and TELCOM program.
* The Model 100 firmware was the last Microsoft product that Bill Gates developed personally, along with Suzuki. According to Gates, "part of my nostalgia about this machine is this was the last machine where I wrote a very high percentage of the code in the product"
