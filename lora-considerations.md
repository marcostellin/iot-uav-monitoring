# Some considerations about LoRa

## Packet size

The payload of the packets is set (in the first simulations) to 10 bytes. According to some sources, sending a pair of GPS coordinates can be achieved with as little as 48 bits [link](https://www.thethingsnetwork.org/forum/t/best-practices-when-sending-gps-location-data/1242/13).
This aspect needs to be studied and evaluated more in the future since the packet size influences a lot the time on air of a packet and consequently the number of packets we can send overall. 