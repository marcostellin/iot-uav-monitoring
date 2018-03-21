# Unmanned Aerial Vehicles

Unmanned Aerial Vehicle or UAV is a general term used to identify all the aircrafts that are able to fly without a pilot. Particular kinds of UAVs are Remotely Piloted Vehicles (RPV) and drones. As the name suggests, RPVs are not fully autonomous and need a complete or partial control of a remote pilot using some sort of control system, for example a joystick. Drone is a term that nowadays is used as a synonym of UAV, but should be more appropriately used to indicate "dumb" UAVs only capable of fairly limited actions and with a restricted set of functionalities (e.g. target drones). 

Typically a UAV system is formed by multiple air vehicles and other additional support units that are used to manage and control the system. A typical UAV system configuration is composed of at least the following elements:

* __Air vehicles__: different types of air vehicles exists depending on the propulsion type: fixed-wing airplanes, rotary-wing and ducted fan;
* __Payload__: the most critical part of the system. Each drone is equipped with a payload that is essential to accomplish the mission for which the drone is deployed. A typical payload is a camera.
* __Ground Control Station (GCS):__ the GCS is usually a vehicle that can transport the drones to the place where they need to operate. The GCS is also equipped with all the necessary instruments to process and display the data received from the drones;
* __Data Link Antennas__: antennas are used to establish a two way communication with the air vehicles. The channel is typically split in two sub channels: a low rate channel for transmitting control commands and receiving feedback from the UAVs and a high rate channel for sensors data (e.g. the video stream). LOS is usually required for the system to operate properly;
* __Ground Support Equipment (GSE):__ this unit is optional, but its use is generally required, especially in larger systems. This unit is used to maintain the UAVs and may contain spare parts and additional fuel or batteries to extend the UAV's on-air time.

A diagram of the system, showing the above mentioned components can be seen in the picture below.

![img](https://image.ibb.co/e0fYQH/uav_system.png)

UAV can be classified, depending on their dimensions, in four categories:

* __Very Small UAVs__: with dimensions ranging from few centimeters up to 30-50 cm. They usually use flapping wings or rotary wings and they can handle a very small payload;7
* __Small UAVs__: one dimension is at least 60 cm and at most 1 - 2 meters long. Most of them are fixed-wing and can be launched manually by the operator;
* __Medium UAVs:__ they are too large to be carried by one person, but smaller than a light aircraft. They can carry a payload of approximately 200 Kg;
* __Large UAVs__: used mostly in the military to carry extremely heavy payloads and for missions that require to fly long distances.

Moreover, a UAV can use different kind of engines:

* __Four-cycle / Two-cycle reciprocating internal combustion engine__
* __Rotary engine__
* __Gas turbines__
* __Electric motor__

All the engines, except for the electric motor that makes use of batteries or solar cells, use traditional fuels such as gasoline, diesel or kerosene. In general fuel engines provide more autonomy to the UAV, but new advancements in battery cells density are increasingly shortening the gap. Among the fuel engines, gas turbines are by far the most efficient and they guarantee less vibrations compared to the other types of engines.

In order to fly with a high degree of autonomy, air vehicles need to be equipped with some sort of controller that regulate, for example, the altitude and the airspeed. This control is usually achieved by using a feedback or closed-loop controller in conjunction with a set of sensors that typically include an altimeter, a speed meter and some sort of positioning device (a GPS for example). The measurements of the sensors are used by the controller to compute the right actuation to adjust the roll, pitch and/or yaw of the vehicle. Typically the control is limited to one axis (roll) or two axis (roll + pitch) to limit the controller complexity. Almost all UAVs are nowadays equipped with a GPS, that can be used to complement the measurements of the other sensors and for accurate path planning.

[ADD PART ABOUT COMMUNICATION WITH UAV]

## References

[1] Introduction to UAV Systems, Fahlstrom and Gleason
