# LPWAN Comparison

The authors in [1] provide a useful comparison of the main LPWAN technologies (NB-IoT, LoRa and Sigfox) based on different application requirements. The authors draw the conclusion that systems requiring Quality of Service (QoS) and low downlink latency at the expense of battery life and network coverage should direct their attention to NB-IoT. However NB-IoT it's still in its infancy and it is not widely deployed. Moreover, the need of LTE gateways in the covered area, limit its applications to urban or semi-urban environments where LTE connectivity is already provided. 

Sigfox and LoRa are well tested technologies already deployed, commercialized and successfully tested, thus making them very attractive as solutions to LPWAN connectivity. Sigfox achieves higher range with a single gateway (up to 40 km) compared to LoRa, but the ultra-narrow band modulation makes Sigfox attractive only for those applications requiring very low data rates and where latency is not an issue. Moreover Sigfox is not tunable, since the network is completely managed by Sigfox or by Sigfox partners.

LoRa seems to be a good compromise between NB-IoT and Sigfox. The big advantage of LoRa, compared to the majority of the other competitors, is that the network can be customized depending on the requirements of the application thus giving more flexibility and embracing a wider range of applications. LoRa is also the only technology among the three that allows the creation of local area networks using LoRa gateways. For this reason, LoRa proves to be particularly useful to create a network in rural and remote areas that network operators have no interest in covering.

For all these reasons, LoRa is considered as a valid IoT network to support the collection of data in disaster scenarios such as the one considered in the present work. However certain LoRa limitations and characteristics might reduce the scope of the envisioned system if certain QoS requirements can not be guaranteed. 

## References

\[1\] [A comparative study of LPWAN technologies for large-scale IoT deployment](https://www.sciencedirect.com/science/article/pii/S2405959517302953)