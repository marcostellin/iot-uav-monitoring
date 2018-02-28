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

The objective of this experiment is to determine the number of simultaneous devices that a single gateway can service and with what QoS in terms of PDR.
At the beginning of each simulation, the SF will be fixed and the simulated area radius is set to 3000 m. In this way all the devices fall in the coverage area of the gateway sitting in the middle of the circular area. The LoRa nodes are fixed and uniformly distributed. 
The number of nodes will vary between 10 and 300. The total simulation time is 10 minutes.
Each node will periodically send a packet to the gateway. Different periods have been tested to check if there's a relation between the periods and the PDR. All the parameters not mentioned in this section are the same as the ones used in the previous experiment.

A number of plots have been made after the simulations: 

![img](https://image.ibb.co/kwSYNc/exp2_10s.png)

![img](https://image.ibb.co/cVh02c/exp2_20s.png)

![img](https://image.ibb.co/bUkbax/exp2_30s.png)

![img](https://image.ibb.co/bE5yoH/exp2_60s.png)

![img](https://image.ibb.co/gGsuTH/exp2_100s.png)

As expected, it's possible to see that the SF, the number of nodes and the sending period all influence the PDR. The best results are obtained with a SF of 7: even when packets are transmitted with high frequency (1 packet / 10 seconds) the PDR remains above 0.9 with 50 nodes on the ground.

A mix of SFs and more gateways could improve the results. 

But what's the main cause of packet loss? Interference from other nodes that are transmitting. LoRa is in fact basically an ALOHA protocol with no coordination mechanism to access the network. If two nodes transmit at the same time the packets are simply lost. It is immediately clear why higher SF means more packet loss: the time on air increases with the SF and thus the probability of collision. If  a high PDR and high frequency is an objective a low SF should be chosen. Of course this comes with some disadvantages, like a reduced battery life(????might not be true) and coverage.

## Experiment #3 - Multiple Gateways

The same scenario of experiment 2 will be run but with the presence of multiple gateways. Prior to the experiments it was hypothesized that PDR would not improve. This hypothesis proved to be wrong. In fact, even if most packets are lost due to interference, a relevant quantity is lost due to the lack of available receiving paths at the gateways. In the simulation each gateway has 8 receiving paths centered around 3 different frequencies. By increasing the number of gateways in the same coverage area, we decrease the number of packets lost  because of unavailable gateways, thus increasing significantly the PDR. The best result are seen with low data rates (SF12 for example) where the presence of more receiving paths increases the possibilities of packet reception in at least one of the gateways.

Some results showing this behavior are shown below.

![img](https://image.ibb.co/hhywsc/exp3_i10_dr0.png)

![img](https://image.ibb.co/nci7yH/exp3_i10_dr5.png)

![img](https://image.ibb.co/n8MGsc/exp3_i30_dr0.png)

![img](https://image.ibb.co/gr2NXc/exp3_i30_dr5.png)

Given the results, the presence of overlapping gateways should be considered if an efficient network needs to be deployed.

## Experiment #4 - Mix of nodes with different SFs

In this experiment we try to mix LoRa nodes with different SFs to determine if the presence of mixed nodes configurations improves the PDR. 
Multiple sub-experiments will be performed.

### Experiment #4.1

* Number of gateways: 1
* Number of nodes: 300
* 150 nodes with DR4
* 150 nodes with DR5

Objective: determine if the overall PDR is better than having 300 nodes with DR5.

The result of the mixed environment are reported below:

 DR1|DR2|Period|PDR (sf1)|PDR (sf2) | PDR (overall) | PDR (DR5 300 nodes)
:-------|:------:|-------:|--------:|-----------:|---------------:|-------------
5 | 4  | 10 | 0.729 | 0.539 | 0.665| 0.542
5 | 4  | 20 | 0.837 | 0.738 | 0.788| 0.738
5 | 4  | 30 | 0.895 | 0.820 | 0.858| 0.822
5 | 4  | 60 | 0.956 | 0.909 | 0.932| 0.913

It seems that the overall PDR improves if half of the nodes use DR4 instead of DR5. 

Let's take a look at the results of running a simulation with 150 nodes all with DR4:

 DR|Period|PDR
:-------|:------:|-------
4  | 10 | 0.558
4  | 20 | 0.726 
4  | 30 | 0.809 
4  | 60 | 0.905

And now with 150 nodes with DR5:

 DR|Period|PDR
:-------|:------:|-------
5  | 10 | 0.727
5  | 20 | 0.853 
5  | 30 | 0.908 
5  | 60 | 0.959

We can clearly see that the PDR of the individual streams remains more or less the same. This experiment clearly shows that spreading factors are othogonal and can thus be used to create different networks working simultaneusly. This fact can be used to split the nodes in classes. Each class will have assigned a specific DR according to the priority of that specific class.


## Conclusions

After running the above mentioned experiments (more experiments may be needed in the future), it's possible to say that:

* PDR is affected by the number of gateways serving an area;
* PDR is affected by the SF used by nodes (low SF means low time-on-air, thus less collisions)
* PDR is affected by frequency of packets sent: clearly high frequency means higher collision probability and reduced probability to find a free gateway















































