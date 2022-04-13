#!/bin/bash
# Build VirtualT-DevTerm Harder Better Faster Stronger on A06
#
# gear 3:
# build time	2m18.713s
#
# gear 6:
# build time	0m58.024s
#
#

# Switch to gear 6 turbo mode
sudo /usr/bin/devterm-a06-gearbox -s 6
make clean
time make -j 6
# Return to gear 3 default mode
sudo /usr/bin/devterm-a06-gearbox -s 3
