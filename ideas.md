# Ideas

## Possible envisioned scenario #1

There are groups of moving nodes. The nodes are kind of clustered. These nodes represent the firemen that generally operates in groups in specific locations. These groups have an entry point on the scenario and possibly a target (a specific area or a set of areas to visit). The UAVs know the entry point and start from there. This initial phase is used to collect the position of the devices that can be covered. 

Knowing the position of some of the nodes, the UAV can run periodically a reinforcement learning algorithm and try to move in the available directions in order to maximize the coverage (number of nodes served, comprising of fixed nodes. Mobile nodes should have a higher preference).

Depending on the number of UAVs, the remaining UAVs should be deployed in order to increase the coverage while keeping the distance from other UAVs. These initial UAVs are the ones collecting data. An additional fleet should be sent to create a mesh WiFi network. By knowing the range of communication of each UAV and the position of the LoRa gateways it may be possible to run a centralized algorithm that determines each time the minimum number of UAVs and their position in order to create a mesh network.

