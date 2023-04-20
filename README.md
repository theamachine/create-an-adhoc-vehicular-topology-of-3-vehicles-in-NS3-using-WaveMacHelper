# Create an adhoc vehicular topology of 3 vehicles in NS3 using WaveMacHelper
This was done through the following steps:  
1. Using 3 nodes (vehicles), with simulation time of 20 seconds.  
2. The vehicles are mobile on a rectangular area according to RandomWalk2dMobility model:  
* The area is a rectangle with bounds 0<x<500 and 0<y<500.
* The speed of the vehicles is a uniform random number between 8 m/s and 13 m/s.  
* Vehicles are initially positioned randomly on the rectangular area. In order to set the initial positions of the vehicles randomly, you need to use “RandomRectanglePositionAllocator”.  '
3. At second 1, vehicle 1 (vehicle with index 0) sends a broadcast message (wave short message) via control channel (CCH):  
* The packet payload of the CCH message is 500 Bytes.  
* The ethernet type protocol is set to 0x88dc which correspond to WSMP.  
* The transmission characteristics of this CCH broadcast message are as follows:  
  - The transmission data rate (wifiMode) is OfdmRate12MbpsBW10MHz.
  - The priority of packets is 7 (the packets priority is a number between 0 and 7, and 7 is the lowest priority)
  - The transmission power level is 10.
  - Hint: A TxInfo object should be defined for configuring channelNumber, dataRate, priority, txPowerLevel.  
4. At intervals of 5 seconds (5, 10, 15, 20), vehicle 1, vehicle2 and vehicle 3 broadcast messages of size 1000 bytes via service channel 1 (SCH1):  
* The priority of the packets sent from vehicle 1 is 0 (highest priority).  
* The rate of the broadcast for vehicle 1 is 27Mbps.  
* The priority of the packets sent from vehicle 2 is 5.  
* The rate of the broadcast for vehicle 2 is 9Mbps.  
* The priority of the packets sent from vehicle 3 is 7.    
* The rate of the broadcast for vehicle 3 is 6Mbps.  
5. All the sent packets should have sequence number that increases iteratively per node at each time stamp.  
6. A callback function should be defined in your simulation wherein you are supposed to print out the sender and receiver’s MAC address, the sequence number and the time stamp.
