# LoRa and LoRaWAN

LoRa (Long Range) is a proprietary modulation technique based on Chirp Spread Spectrum (CSS) originally developed by Cycleo and then acquired by Semtech in 2012. As many other LPWAN technologies, LoRa uses the unlicensed sub-GHz bands (868 MHz in Europe, 915 MHz in North America, 433 MHz in Asia). Since the LoRa modulation is proprietary, no official comprehensive description of the protocol exists as of today, but many attempts to reverse engineer the protocol have been performed [2 - 5].

LoRaWAN is an attempt to define a network structure and a MAC protocol for the LoRa technology. The LoRaWAN standard is managed by the LoRa Alliance and it's available for consulting in the LoRa Alliance website [1].

## LoRa Chirp Spread Spectrum

LoRa modulation is a variation of the CSS modulation technique, which is in turn a subcategory of DSSS. In DSSS the data is spread over a wider bandwidth by multiplying the transmitted symbol by a chip sequence of Pseudorandom Noise (PN) generated with a frequency that is higher than the symbol generation frequency. The result is a signal with a very low SNR, practically unrecognizable from background noise and very resilient to RF noise. The PN sequence, known also by the receiver, is used to de-spread the signal and recover the original data. Therefore DSSS presents some ideal characteristics for being used in a LPWAN solution. Unfortunately, DSSS requires a highly accurate and expensive clock source to synchronize the PN sequence at the transmitter and at the receiver. This fact makes traditional DSSS not particularly suited for low-cost end devices such as the ones deployed in a LPWAN.

CSS is a variation of DSSS initially used only for radar systems and for a long time ignored in communication systems, but recently revalued due to its robustness to a wide range of channel degradation mechanisms such as multipath fading, signal fading, Doppler shift and jamming interferers.

In CSS the bandwidth spread is obtained not by using a PN sequence, but by using chirps, i.e. signals of continuously increasing or decreasing frequency inside a predefined frequency interval. An increasing frequency chirp is called up-chirp, while a decreasing frequency chirp is called down-chirp. The increase/decrease in frequency is, like in LoRa, typically linear to simplify the transmitter and receiver hardware. 

In LoRa, a symbol is first chipped at a higher frequency that depends on the Spreading Factor (SF) and then modulated into a chirp. The chirp is characterized by the SF, a minimum frequency ![equation](http://latex.codecogs.com/gif.latex?f_%7Bmin%7D) and a maximum frequency ![equation](http://latex.codecogs.com/gif.latex?f_%7Bmax%7D). An initial frequency ![equation](http://latex.codecogs.com/gif.latex?f_0) is assigned to the symbol and then the chirp sweeps the frequencies from ![equation](http://latex.codecogs.com/gif.latex?f_0) to ![equation](http://latex.codecogs.com/gif.latex?f_%7Bmax%7D), wrapping around from ![equation](http://latex.codecogs.com/gif.latex?f_%7Bmax%7D) to ![equation](http://latex.codecogs.com/gif.latex?f_0) when hitting the end of the available bandwidth ![equation](http://latex.codecogs.com/gif.latex?B%20%3D%20f_%7Bmax%7D%20-%20f_%7Bmin%7D). The period of the chirp is determined by the following formula:

![img](http://latex.codecogs.com/gif.latex?T_c%20%3D%20%5Cfrac%7B2%5E%7BSF%7D%7D%7BB%7Dsecs)

The modulation data rate can be computed as:

![img](http://latex.codecogs.com/gif.latex?R_b%20%3D%20SF%20*%20%5Cfrac%7B1%7D%7BT_c%7D%20bits/s)

Therefore a bigger SF corresponds to a longer chirp period and consequently a lower data rate. On the other side a higher SF makes the signal more resilient to noise and other RF interferers, thus permitting a lower receiver sensitivity and therefore the possibility of reaching higher distances. This robustness is counterbalanced by a higher probability of collision, since the signal takes significantly more time to be transmitted. 

In LoRa the SF can vary between 7 and 12. If the coding rate is not considered, the data rates computed with the above formulas are the following:

```
SF = 12, B = 125 kHz, BitRate = 250 bps
SF = 11, B = 125 kHz, BitRate = 440 bps
SF = 10, B = 125 kHz, BitRate = 980 bps
SF =  9, B = 125 kHz, BitRate = 1760 bps
SF =  8, B = 125 kHz, BitRate = 3125 bps
SF =  7, B = 125 kHz, BitRate = 5470 bps
```

## LoRa Physical Layer Packet

Through a process of reverse engineering explained in [6], it was possible to determine how the physical layer LoRa packet is composed. Three fields are defined:

* __Preamble__: used for transmitter and receiver synchronization. It's composed of a variable sequence of up-chirps having the same duration;
* __Start of Frame Delimiter (SFD):__ indicates the beginning of a frame. It's composed of 2 down-chirps;
* __Header and data:__ up-chirps of varying length. The data is extracted from the instantaneous frequency transitions of the chirps.

Some additional techniques are used to increase the robustness of the LoRa modulation:

* __Gray indexing__: before being transmitted, the symbols are Gray indexed. Gray indexing is a way of encoding digits such that two consecutives digits have a change only in one bit position. This allows for more error tolerance. In the actual implementation, the symbols are read as gray-coded and de-grayed before transmission. Therefore the decoder needs to gray-code the symbols to recover the original symbols;
* __Data whitening__: this process induces randomness in the sequence of bits in order to reduce long sequences of repeating bits. This allows a better clock recovery by the receiver. The whitening is done by XORing the symbols with a pseudorandom sequence that is known by both the transmitter and the receiver;
* __Interleaving:__ the initial ordered sequence of bits is scrambled using a scrambling sequence. The receiver uses the inverse operation to reconstruct the original bit sequence. This technique is typically used in conjunction with forward error correction to spread multiple consecutive errors over the whole frame. LoRa uses a diagonal interleaver with some variations;
* __Forward Error Correction (FEC)__: enables bits damaged during the transmission to be recovered at the receiver by introducing some redundancy in the transmitted data. LoRa implements a Hamming FEC that substitutes a 4-bit word with a codeword with length in the range [5-8] bits. A longer codeword provides additional protection at the cost of additional bits. When an Hamming (5,4) or (6,4) is used, only error detection is available. A Hamming (7,4) can correct a single bit error, while a Hamming (8,4) can correct a two bit error.

To improve reliability, code diversity is also exploited. Different spreading factors use semi-orthogonal codes, thus enabling the recovery of two frequency and time overlapping signals if they don't share the same SF. This characteristic enables the network to achieve a higher overall throughput, due to the fact that colliding packets with different SF are still correctly received.

## The LoRaWAN standard

The LoRaWAN standard [7], promoted by the LoRa Alliance, defines the network architecture and the MAC layer of a system using the LoRa modulation. A brief overview of the main LoRaWAN characteristics and features is given in the following sections.

### Architecture and Topology

The overall architecture of LoRaWAN is shown in the picture below:

![img](https://image.ibb.co/d4bUxx/Lo_Ra_Network_Architecture.jpg)

LoRaWAN implements a star-of-stars topology interconnecting five different network elements:

* __End Devices (ED)__ : they are typically low-powered devices with sensing capabilities. They transmit/receive data using the LoRa radio channel. They can establish communication only with LoRa gateways. No pairing with the gateways is necessary: EDs transmit the message assuming that at least one gateway is listening;
* __Gateways (GW)__ : one or multiple gateways are responsible for collecting and aggregating the data coming from the end nodes. Gateways are responsible for demodulating the LoRa uplink transmissions and relaying the uplink messages to a network server using a traditional IP-based backhaul network. Upon requests, GWs can also send downlink messages to end nodes;
* __Network Server (NS)__: collects the packets coming from the gateways and routes them to the appropriate application server. The NS is also responsible of sending/receiving MAC commands and discard duplicated packets;
* __Application Servers (AS)__: implement the application specific logic. An AS aggregates, analyzes and/or stores data and present it to the end users in a meaningful way. End users only interact with the services offered by an AS;
* __Join Server (JS)__: manages end devices activation and derives and stores the keys used by the network for encrypting operations.

### End Devices Classes

The LoRaWAN stack is summarized in the picture below:

![img](https://image.ibb.co/dxet7x/lora_stack.png)

The LoRaWAN specification defines three classes of ENs with different characteristics and functionalities: Class A EDs (baseline), Class B EDs (beacon) and Class C EDs (continuous). All the EDs must implement Class A operation mode and may optionally implement Class B or Class C operation modes. All the classes offer bi-directional communication capabilities.

* __Class A__: it's the default operation mode of a LoRaWAN device. In fact, the switching to Class B or Class C is only done after the ED joins the network by using specific MAC commands. Class A EDs open two short receive windows only after an uplink transmission, thus making this mode of operation best suited for applications that need to communicate with the end devices only after an uplink. The first receive window (RX1) is opened immediately after the uplink transmission and uses a frequency and data rate that are functions of the uplink transmission. The second receive window (RX2) uses instead a fixed data rate and frequency that can be configured using MAC commands. Both windows must stay open at least for the time needed for a GW to detect the downlink preamble. A window can't be closed until the eventual downlink transmission is completed. Thanks to this limitations in the downlink, class A devices are the most power efficient;
* __Class B__: this operation mode adds synchronized reception windows in the ED. In addition to the two receive windows of Class A devices, additional receive windows can be opened at specific time instants. In order for Class B devices to work, GWs must periodically send a beacon that specifies the exact time at which the windows must be opened. This mechanism enables more flexibility in sending downlink messages, representing a good compromise in terms of EDs power consumption and EDs reaction time to downlink messages;
* __Class C:__ the receiving windows are continuously open and only closed when the ED is transmitting. Class C is the least power efficient and should be used only in EDs with relaxed power constraints that need low latency downlink messages.

### MAC Frames and MAC commands

![img](https://image.ibb.co/dSXf4c/lora_packets.png)
An overview of the PHY and MAC packets structure is shown in the above figure. 

MAC frames are encapsulated in the PHY payload (PHYPayload) and are composed of a MAC header (MHDR), a MAC payload (MACPayload) and a Message Integrity Code (MIC). The MHDR contains, among other fields, a 3-bit MType field that specifies the message type. The available types are shown in the table below:

![img](https://image.ibb.co/gCtnjc/mac_message_types.png)

Join-request, Join-Accept and Rejoin-request are used during the Over The Air (OTA) procedure described in the next section, while data messages are used for transmitting MAC commands and/or application data. Data messages may require or not an acknowledgement from the receiving device (an ED or a GW). The MACPayload field is additionally divided in frame header (FHDR), frame port (FPort) and frame payload (FRMPayload). The FHDR contains the 4-byte device address (DevAddr), a 1-byte frame control (FCtrl) used to request the Adaptive Data Rate (ADR) feature and the need of acknowledgement in case of confirmed data messages,  a 2-byte frame counter (FCnt) and an optional frame option (FOpts) containing up to 15 bytes of MAC commands. The 1-byte FPort field is mandatory and it is used to specify an application specific port in the range 0-224 (ports 225-255 are reserved), where the port 0 can be used only when the frame only contains MAC commands. In this case, the commands are not piggybacked in the FOpts field, but directly inserted in the FRMPayload. Finally the MIC field is used to check the integrity of all the fields composing the PHY payload.

MAC commands are simple commands exchanged exclusively between the NS and the EDs. The commands can be piggybacked to a data message using the FOpts field or they can be carried in the FRMPayload by specifying a FPort with value 0. Each command is defined by a 1-byte Command identifier (CID) followed by zero or more command specific octets. MAC commands can be used, for example, to:

* Check the line connectivity;
* Limit ED duty cycle;
* Set the receive parameters of RX2;
* Request status info to EDs;
* Set the delay between a transmission and the opening of the first receive window;
* Change ADR parameters;
* Request current time and date to the network.

### Adaptive Data Rate

LoRaWAN specifies an ADR mechanism that is used to optimize the data rate, transmission power and spectrum usage of EDs depending on the quality of the link. This typically means increasing the SF to reduce the SNR or decreasing it to minimize power consumption and on-air time when the SNR is well above the threshold. An ADR request always comes from the ED by setting the ADR bit in the FCtrl field. This bit signals to the NS that the node is willing to accept ADR parameters variations based on the observations of the NS itself. It's worth noting that the specification doesn't specify an algorithm that must be used for ADR, thus leaving the implementation details to the manufacturer of the network equipment or the network operator. The ADR, as per specification, should only be used by static nodes or by "smart" mobile nodes that are able to detect when they are parked for an extended period of time. 

### End Device Activation

Before being able to participate in the network, each ED must join the network using either Over-The-Air Activation (OTAA) or Activation By Personalization (ABP) procedures. These procedures have the purpose of deriving the session keys that are necessary to encrypt the communication between the ED and the NS.

In OTAA, before starting the join procedure, an ED requires a Device Extended Unique Identifier (DevEUI) that uniquely identifies the device, a Join EUI (JoinEUI) that uniquely identifies the JS that helps in the join procedure, an Application Key (AppKey) and a Network Key (NwkKey). The keys are 128 bits AES keys and must be stored in a secure way in the ED. The join procedure is carried out by exchanging a Join-Request and a Join-accept between the ED and the JS, during which the session keys are derived by using the AppKey and the NwkKey. The session keys are then used to encrypt the MAC payload using AES. During the procedure a network specific Device Address (DevAddr) is also assigned to the ED.

The ABS procedure skips the Join-request / Join-accept message exchange, thus leaving to the network operator the responsibility of loading the sessions keys and the DevAddr in the ED.

## References

\[1\] [LoRa Alliance](https://www.lora-alliance.org/)

\[2\] [Reversing LoRa](https://static1.squarespace.com/static/54cecce7e4b054df1848b5f9/t/57489e6e07eaa0105215dc6c/1464376943218/Reversing-Lora-Knight.pdf)

[3] Chirp Spread Spectrum as a Modulation Technique for Long Range Communication

[4] LoRa Modulation Basics by Semtech	

[5] Dedicated networks for IoT : PHY / MAC state of the art and challenges

\[6\] Decoding LoRa: Realizing a Modern LPWAN with SDR

[7] LoRaWAN specifications v1.1

