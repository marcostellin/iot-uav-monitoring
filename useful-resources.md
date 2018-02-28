# MANETs and Drones

In this section, an overview of possible solutions to the establishment of a MANET WiFi network between nodes is given.

## Resources

* [UAV Relay Network to Support WSN Connectivity ](ftp://ftp.inf.ufrgs.br/pub/simoo/papers/icumt10.pdf)

An approach based on beacons. Nodes on the ground are fixed and clustered.

* [UAV-Assisted Disaster Management: Applications and Open Issues](https://www.archives-ouvertes.fr/hal-01305371/document)

Disaster management and logistic application

> The goal of this set of applications in disaster management is to gather the information during the disaster phase, especially regarding the movement of the people endangered by the disaster, as well as the rescue teams deployed on the disaster area.

UAV localization

> Approaches like __ant-foraging__ algorithms that reinforce paths based on availability/number of mobile pings can drive the UAVs towards the locations with high density of survivors.

Creating and maintaining a relay network

> A two stage process is needed: an initial round of centralized determination of optimal relay points (which we call anchors) that connect the disaster region to the nearest radio access network is followed by a round of de-centralized correction during deployment.

* Reference [8] of previous link: [Enabling Mobility in Heterogeneous Wireless Sensor Networks Cooperating with UAVs for Mission-Critical Management](http://epdoc.utsp.utwente.nl/64671/1/IEEE_WCM_AysegulTuysuz.pdf)

  > The use of mobile sensor nodes could provide significant improvements. They can provide the ability to  closely monitor the objects that we want to guard in WSN and  to  look  at  the  events  at  a  smaller  granularity than  static nodes.  

> Other mobile sensors  are  the  nodes  on  the  firefighters. Firefighters  carry  a  set  of  (i) physiological,  (ii) kinetic-(acting),  and  (iii) environmental sensors.  These  sensors  on the  firefighters  form  a body  area  network (BAN)  which consists of a set of mobile and compact intercommunicating sensors, either wearable or implanted into the human body. 

Three types of data:

* __Streams__: they  produce  data (e.g.   temperature,   humidity,   etc.)   continuously,   often   at defined time intervals, without having been explicitly asked for that  data.  This  kind  of  data  does  not  need  to  be  transferred reliably since it has generally the same content as the previous reading.
* __Critical data__: coming from the event region or the body   area   network   on   firefighters.   The   critical   data transmission  should  be  reliable  and  in  a  timely  manner  for mission-critical applications. 
* __Commands__: from data sinks (UAVs) to ground nodes to change their operation mode.

Critical QoS factors:

* Latency
* Reliability



* [DistressNet: A Wireless Ad Hoc and Sensor Network Architecture for Situation Management in Disaster Response](http://ace.eecs.ohiou.edu/~chenji/docs/10-commag-dnet.pdf) [TO READ]
* [Multi-Agent System Based Wireless Sensor Network for Crisis Management](http://ieeexplore.ieee.org/document/5683166/) [TO READ]

## Reinforcement Learning

[Autonomous  UAV  Navigation  Using  Reinforcement  Learning](https://arxiv.org/pdf/1801.05086.pdf)

May be promising to use RL to maximize nodes coverage (higher reward for mobile nodes). 

> In  each  state,  the  UAV  can  take  an  action a_k from a  set  of  four  possible  actions A:  heading  North,  West, South  or  East  in  lateral  direction,  while  maintaining  the same  altitude.





