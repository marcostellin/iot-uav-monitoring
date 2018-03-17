# LPWAN Technologies

In this section a brief overview of the main LPWAN solutions and protocols is given. A detailed description of LoRa and LoRaWAN protocols will be given in the next chapter.

## Sigfox

Sigfox is a an LPWAN patented technology and a communication service provider that offers an end-to-end solution to connect hundreds of devices to the Internet. Sigfox operates in the ISM bands (868 MHz in Europe, 915 MHz in North America and 433 MHz in Asia) using a bandwidth of approximately 200 kHz. The signal is transmitted using an ultra-narrow band Binary Phase Shift Keying (BPSK) modulation, that guarantees low power consumption, low noise levels and a high receiver sensitivity. Each message is 100 Hz wide and is transferred with a maximum speed of 100 bps. The uplink payload size is fixed and equal to 12 bytes. Considering the mandatory duty cycle of 1% in the ISM bands, each device is able to transmit at most 140 messages/day. The access to the medium is random, with no synchronization between devices. To increase the reliability and avoid collisions, each message is transmitted three times and each replica is transmitted on a different time and frequency. The receivers listen on all the 200 kHz spectrum for ultra-narrow band signals. The reception is cooperative: the transmitted message is received by all the base stations in the range of the device (generally three), thus improving the reliability of the service. The Sigfox network is bi-directional, but with a strong asymmetry. A downlink message can only be delivered in a window that is optionally opened immediately after an uplink transmission and, due to duty cycle limitations, only 4 downlink messages/day with a fixed payload of 8 bytes are guaranteed. 

The Sigfox network is based on a simple star topology: end-devices transmit the messages to the base stations and the base stations relay the messages to the proprietary Sigfox backend, where callbacks to a private cloud server can be configured.

Some degree of security is provided by default using authentication and AES encryption. Payload encryption is offered as an optional feature.

## NB-IoT

The 3GPP introduced a new LPWAN technology called Narrowband IoT (NB-IoT) in the LTE Rel-13 specification. NB-IoT is based on LTE with some simplifications and optimizations to support low-power long-range communication. Thanks to the common characteristics with the LTE technology, the implementation of NB-IoT can be simply achieved with a software upgrade of existing LTE equipment, thus reducing the costs and the time to reach the market.

 NB-IoT operates on the same licensed frequencies used by LTE and GSM using a 180 kHz bandwidth, further divided in 12 subcarriers of 15 kHz in downlink and 12 or 48 subcarriers in uplink. Frequency Division Duplex (FDD) mode is employed to support simultaneous uplink and downlink transmission and reception. Frequency Division Multiple Access (FDMA) is used in uplink, while Orthogonal FDMA (OFDMA) is used in downlink. The modulation is a Quadrature PSK (QPSK). NB-IoT supports three operational modes:

- In-band: one or more LTE Physical Resource Blocks (PRB) are reserved for NB-IoT;
- Guard-band: the guard-band of the LTE carrier is used;
- Stand-alone: one or more GSM carriers are reserved for NB-IoT.

The maximum allowed payload of a packet is 1500 bytes. A maximum throughput of 200 kbps in downlink and 20 kbps in uplink can be achieved.

## Ingenu

Ingenu is a company offering an end-to-end LPWAN solution based on the patented Random Phase Multiple Access (RPMA) Direct Sequence Spread Spectrum (DSSS) technology. Contrary to all the other LPWAN solutions, Ingenu RPMA uses the unlicensed 2.4 GHz frequency band. This choice has the advantage of guaranteeing worldwide interoperability of the devices and taking advantage of the little regulatory requirements imposed by the authorities in this frequency band (no duty cycle limitations for example). RPMA is a variation of Code Division Multiple Access (CDMA) in which the devices start the transmission at a random time in the time slot. The receiver employs multiple demodulators to decode the signals arriving at different points in time. RPMA is only used in uplink, while in downlink the traditional CDMA is used. Ingenu uses Time Division Duplex (TDD) to separate uplink and downlink transmissions. The employed modulation is Differential BPSK (D-BPSK). Thanks to the RPMA technology, receivers are reported to achieve a sensitivity as low as -145 dBm. 

## Weightless

Weightless is a set of LPWAN standards developed by the Weightless-SIG special interest group. Three standards have been developed until now: Weightless-W, Weightless-N and Weightless-P. In this section only the most recent Weightless-P standard is considered.

Like most LPWAN solutions, Weightless-P operates in the sub-GHz license-exempt bands. It offers bi-directional, fully acknowledged communication to improve reliability. TDD is used and the signal modulation is Orthogonal QPSK (OQPSK)  both in uplink and in downlink. In uplink a combination of FDMA and TDMA is used using a bandwidth of 12.5 kHz or 100 kHz for scheduled uplink transmissions. This bandwidth allocation allows for a maximum uplink data rate of 10 kbps (12.5 kHz) or 100 kbps (100 kHz). Frequency hopping is also used to limit interference. In downlink just TDMA is used since base stations can coordinate their transmissions.   



\[1\] [Sigfox Overview](https://www.disk91.com/wp-content/uploads/2017/05/4967675830228422064.pdf)

\[2\] [Overview of Narrowband IoT in LTE rel-13](http://ieeexplore.ieee.org/document/7785170/?denied)

\[3\] The RPMA e-book

[4] Weigthless-P 1.03 specifications

