# 5G-VANET
The codes for [Software-Defined Cooperative Data Sharing in Edge Computing Assisted 5G-VANE](https://ieeexplore.ieee.org/abstract/document/8897045). 

# Requirements: software
NOTE: Only tested on NS3.21!
1.	Requirements for SUMO and NS3.21;
If you have any problem on installation and compiling, please referred to [NS3](https://www.nsnam.org/documentation/) and [SUMO](http://sumo.dlr.de/index.html).
# Requirements: hardware
NS3 can be executed both in Windows and Linux system. However, our operating system for 5G-VANET system is Linux (Ubuntu 14.04 LTS)
# Installation
1.	Clone the 5G VANET repository;
2.	Replace the WiFi module with this new module;
3.	Install two module cgame and cgamemac (the codes are located in src/)
4.	Compile the NS3.21 environment by following command:

    ./waf build

5.	Run the demo and algorithm by following command:

    ./waf --run scratch/graphtheoryGreedy

6.	The simulation results will be saved into a text file.  
