# IoT-UAV-Monitoring

## Introduction

Forest fires are among the most destructive events that can happen in nature. The losses, in terms of environmental resources and human lives are often extremely high. Portugal, Spain and Italy are just few examples of countries where wildfires occours multiple times a year. In most of the cases wildfires are caused by the uncareful behaviour of humans and just a small percentage is caused by natural reasons [CIT].
With the advent of new technologies and communication protocols, it's possible to address this problem and react more swiftly to wildfires. In particular the Internet of Things (IoT) and Unmanned Aerial Vehicles (UAVs), most known as drones, are the most promising technologies due to the low cost of deployment and high energy efficiency. 
A suitable communication network needs to be established in order to collect the data that sensors produce. In urban areas this can be easily achieved by using existing networks (e.g. cellular network, Sigfox, etc.) or networks specifically deployed for this purpouse.
However, most of the times, forests are located in rural areas where the coverage of such networks is limited or even non existent. 

## Objectives

In the present work a new approach to provide network coverage to remote areas is studied and validated. 
In this approach one or multiple UAVs are deployed in the relevant area in order to form a WiFi mesh network supporting the communication with a gound station. Each UAV is equipped with a LoRa gateway that allows the LoRa nodes on the ground to send realtime updates to the ground station. This system is more specifically addressed to collect data coming from moving nodes (such as firemen, robot sensors, etc..). For this reason UAVs needs to adjust their position in order to provide the best coverage to the nodes regardless of their position and the data should be reliably forwarded to the ground station.

The scenario is a forest where one or multiple wildfires have been detected. The forest will contain both fixed nodes and moving nodes. Moving nodes will have the priority in terms of coverage.

## Tools

The quality of the solution will be evaluated by simulating the scenario using the [ns-3](https://www.nsnam.org/) network simulator with the [LoRa module](https://github.com/DvdMgr/lorawan) and the WiFi module.