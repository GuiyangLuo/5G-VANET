# 5G-VANET
The codes for 5G assisted software defined vehicular network for cooperative data sharing. This code is only for peer review! Once the paper is accepted, we will update this repository.

# Requirements: software
NOTE Only tested on NS3.21!
1.	Requirements for SUMO and NS3.21;
If you have any problem on installation and compiling, please referred to [NS3](https://www.nsnam.org/documentation/) and [SUMO](http://sumo.dlr.de/index.html).
#  Requirements: hardware
NS3 can be executed both in Windows and Linux system. However, our operating system for 5G-VANET system is Linux (Ubuntu 14.04 LTS)
#  Installation
1.	Clone the 5G VANET repository;
2.	Replace the WiFi module with this new module;
3.	Install two module cgame and cgamemac (the codes is located in src/)
4.	Complie the NS3.21 environment by following command:

    ./waf build

5.	Run the demo and algorithm by following command:

    ./waf --run scratch/ graphtheoryGreedy

6.	The results has been saved to the txt files after the simulation. 
