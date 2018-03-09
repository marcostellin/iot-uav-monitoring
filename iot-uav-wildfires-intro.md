# Internet of Things: Wireless Sensor Networks and Body Area Networks for Firefighting

## The Internet of Things

In the last years, the availability and popularity of the Internet infrastructure has given prominence to a new paradigm called Internet of Things (IoT). This word is used nowadays to describe a broad range of technologies and communication architectures and there is still not a clear consensus on the exact definition of this term.

According to IEEE the Internet of Things can be defined as [1]:

> Internet of Things envisions a self configuring, adaptive, complex network that interconnects "things" to the Internet through the use of standard communication protocols. The interconnected things have physical or virtual representation in the digital world, sensing/actuation capability, a programmability feature and are uniquely identifiable. The representation contains information including the thing’s identity, status, location or any other business, social or privately relevant information. The things offer services, with or without human intervention, through the exploitation of unique identification, data capture and communication, and actuation capability. The service is exploited through the use of intelligent interfaces and is made available anywhere, anytime, and for anything taking security into consideration.”

If the above definition is considered, the IoT is characterized by:

* __Uniquely addressable devices__: the devices have a unique address in the network and have sensors or actuators to interact with the surrounding environment;
* __Devices offer services__: the interconnected devices offer a service through their sensing and actuating interfaces;
* __Connection to the Internet__: the devices, through the direct or indirect connection to the public Internet network, can be accessed anywhere, anytime.

The typical IoT architecture can be logically divided in three layers:

* __Perception Layer__: this layer is responsible for sensing objects and collecting information from the surrounding environment. It is composed of different kind of sensors, depending on the application of the IoT system;
* __Network Layer__: deals with the communication of the information retrieved by the perception layer to one or multiple central nodes by means of appropriate communication protocols. The network layer may be a mobile network, the Internet, radio, satellite, etc.
* __Application Layer__: provides a customized service to the end users by analyzing data coming retrieved by the perception layer. The data is then displayed in order to maximize the end user experience.

This broad definition of IoT includes an equally broad range of possible applications: environment monitoring, health monitoring, smart grids systems, home automation, etc.

For the purpose of this thesis, only environment, health and position monitoring applications for wildfire prevention and firemen support are considered. For a more comprehensive overview of the possible applications, we refer to [2].

## Wireless Sensors Networks

A Wireless Sensor Network (WSN) is a network of devices equipped with one or multiple sensors that communicate using an energy-efficient communication protocol.

WSNs have been thoroughly studied and investigated in the literature for the purpose of forest fire detection and monitoring. Previous solutions, such as CCD cameras, satellite imagery and optical, infrared and thermal images have proven to be unreliable due to, respectively, the cost of the infrastructures needed (towers), the long scan cycle and the sensibility to weather conditions (especially fog and clouds) and a high rate of false positives. WSNs, when deployed in vast forest areas, have proven to be an efficient, scalable and cost-effective way of gathering useful data that can help preventing and forecasting forest fires. Energy efficiency is another primary concern in such scenarios, since all the devices are battery-powered and densely deployed, thus making maintenance operations very difficult. For this reason most of the solutions use short-range energy-efficient communication protocols of the family 802.15.4 (e.g. ZigBee) and devices alternate between an active and sleeping state to save energy. Energy harvesting is also a common solution to increase battery life. Forest areas are usually far away from cities or urban environments, thus restricting the access to a reliable network infrastructure such as WiFi or even cellular networks. In these cases the preferred approach relies on sensors that can self-organize and can set up an ad-hoc wireless network. Messages are then routed to a central node using a multi-hop routing protocol.

One possible WSN solution using this approach for detecting wildfires is presented in [3]. The authors propose a WSN composed of devices mounting humidity and heat sensors, with the addition of gas/smoke sensors. The retrieved data is used to compute the probability of fires in the area and to detect the position of the fire edge when the fire is close to the the sensors. Devices communicate using ZigBee using a multi-hop routing protocol. The information retrieved by sensor nodes is gathered in central nodes mounted in an elevated position. These nodes are more powerful and can communicate with the base station using the long range Industrial, Scientific and Medical frequency band. A similar solution is presented also in [4] and [5].

Even tough WSNs using multi-hop routing protocols work, a particular attention should be used when deploying the sensors: a sensor placed too far from the others may not be able to relay the data to the central node. Different environments conditions, influencing the signal propagation, can make the decision of how many sensors and where to place them very difficult. Even tough retrieving data from WSNs is not the primary concern of our system, the ability of getting data from such systems could be a useful secondary feature especially if the WSN network infrastructure has been compromised by the fire.

## Wireless Body Area Networks

Wireless Body Sensor Area Networks (WBSAN) are networks of devices equipped with different kind of sensors that communicate over a very short distance using a wireless connection. Sensors are typically small since they need to be positioned on the body or on equipment placed in the body without interfering with the activities of the holder. WBSANs found their main applications in health monitoring, where various kinds of physiological conditions can be monitored in real-time.

WBSANs are particularly fit to support the activities of firefighters during emergencies. Usually an emergency expects the participation of one or more firemen teams coordinated by an Incident Commander (IC). The IC manages the operations from a safe position and communicates with the firemen on the ground using radio communications. Firemen provides the IC with all the relevant information, such as environment conditions, their positions or dangerous situations, but this capability is limited by what the fireman can see and perceive and is bound to human errors. Relevant data may instead be sent to the IC by a WSAN installed in the firemen body or equipment. 

An international survey on many fire departments described in [6] shows that firemen would greatly benefit from the next generation of smart equipment. At the top of the list of features that firemen require the most is a system to monitor their position in real-time at any moment during the operation. Monitoring of noxious gases, heat, heart rate, blood pressure and body temperature come later in the list. All these functions can be carried out by a properly configured WSBAN. In fact in [7] the authors proposes a comprehensive WSBAN to monitor different health parameters of firefighters along with the position in an indoor environment using the RSSI of the signal. In the outdoor environment considered in this project, a more precise GPS module can be used instead.

Given the importance of communicating such parameters, it's of utmost importance for our system to be able to receive the data coming from the firemen WSBANs and relay it to the IC outpost in a timely manner. 



\[1\] [Towards a definition of IoT](https://iot.ieee.org/images/files/pdf/IEEE_IoT_Towards_Definition_Internet_of_Things_Revision1_27MAY15.pdf)

\[2\] [Internet of Things (IoT): A vision,architectural elements,and future directions](https://www.sciencedirect.com/science/article/pii/S0167739X13000241)

\[3\] [Forest Monitoring and Wildland Early Fire Detection by a Hierarchical Wireless Sensor Network](https://www.hindawi.com/journals/js/2016/8325845/)

\[4\] [Forest Fire Modeling and Early Detection using Wireless Sensor Networks](https://www.cs.sfu.ca/~mhefeeda/Papers/ahswn09a.pdf)

\[5\] [Forest Fire Monitoring System Based On ZIG-BEE WirelessSensor Network](https://www.researchgate.net/publication/226288294_Forest_fire_detection_system_based_on_ZigBee_wireless_sensor_network)

\[6\] [What do firefighters desire from the next generation of personal protective equipment? Outcomes from an international survey](https://www.ncbi.nlm.nih.gov/pmc/articles/PMC4591136/pdf/indheaoth-53-434.pdf) 

\[7\] [Comprehensive monitoring of firefighters by a Wireless Body Area Sensor Network](http://ieeexplore.ieee.org/document/6616245/)







