# Experiments

A series of experiments using the ns-3 network simulator, the LoRa module and the WiFi module will be performed in order to evaluate specific aspects of the involved networks. These experiments are just a preliminary work to understand the capabilities of the ns-3 simulator and of the modules that will be used when more serious simulations will be performed.

## Experiment #1 - LoRa Gateway Coverage

The objective of this experiment is to determine the maximum coverage of a LoRa gateway as a function of the SF used. Additionally also the throughput and the delay are computed for each SF.



Scenario Parameters: 

* Propagation model : Alfa Friis with alfa = 3.29;
* Signal propagation speed: light speed;
* Drone altitude: 60 m;
* LoRa Node Altitude: 1m;
* LoRa Node speed : constant 1 m/s along the x axis;
* Interval between packets : 10 s;
* Transmission Power: 14dBm;
* Packet size: 19 bytes;

The obtained results are:

SF|Delay (ms)|Throughput (bit/s)|Max coverage (m)
:-------|:------:|-------:|--------
12  | 1319  |115.24  |6309
11  | 659   | 230.49 | 5329
10  | 330  |  460.99 | 4529
  9 | 185 | 820.10  | 3789
8   | 103 | 1476.99 | 3189
7   | 51.4 | 2953.97 | 2679

The results are as expected. The coverage increases proportionally to the SF, but on the other side the throughput decreases and the delay between the tranmission of a packet and it's reception increases as well.
Given the critical function of our infrastructure, a SF of 7, 8 or 9 may be desirable. It's also necessary to take into consideration that two packets with the same SF results in a collision, so a mix of spreading factors may be desirable to achieve a good PDR.

## Experiment #2 - LoRa Gateway devices support

The objective of this experiment is to determine the number of simultaneous devices that a single gateway can service and with what QoS in term of PDR.
At the beginning of each simulation, the SF will be fixed and the simulated area radius will be set according to the results obtained in Experiment #1. In this way all the devices should fall in the coverage area of the gateway sitting in the middle of the circular area. The LoRa nodes are fixed and randomly distributed. 
The number of nodes will vary between 
Each node will periodically send a packet to the gateway. The period will be set to 10 seconds and eventually increased.
