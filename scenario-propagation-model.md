# Scenario 

In order to perform an accurate simulation, a good scenario must be considered. As mentioned in the Introduction, the simulations will take place in a forest covered by trees. Initially we consider a flat forest where all the ground nodes are at the same altitude. Future simulations may consider more nodes at different altitudes (e.g.nodes on the trees or surface with hills). 

Some parameters needs to be defined and taken into account:

* Adequate propagation model in presence of trees;
* Average height of the trees: drones have no sensors and have to fly above the trees;
* Dimensions of the simulated area.

The above mentioned parameters, once defined, will be used throughout the simulations. Let's address each parameter individually.

## Propagation Model

In presence of foliage in the path of the signal, the signal undergoes some additional attenuation that depends on the depth of the foliage. An appropriate path loss model must be chosen to take this effect into account.
Multiple models have been proposed in the literature and they are generally divided into empirical models and analytical models. In general, empirical models are considered more reliable than analytical models. 

### Modified Exponential Decay

For the purpose of this work, the work in [1] is considered. The authors compare the different propagation models and state that the most reliable in many different circumstances is the Modified Exponential Decay Model (MED) and gives the parameters for different forest scenarios. 
The MED model is as follows:

![img](http://latex.codecogs.com/svg.latex?PL%3DXf%5EYd%5EZ)

where _f_ is the frequency in MHz and _d_ the depth of the vegetation in meters. The authors estimated the best parameters values: X = 0.75; Y=0.25; Z=0.35.

This model needs to be combined with a suited propagation model when the signal pass the vegetation. As stated in [2] the best model is the Friis Propagation Model.

For the communication between the UAVs a Friis model is also considered since no obstacles are present between the UAVs.

### Alfa-Friis

The model described in [4] is also considered as an alternative model. The model is a variation of the Friis model where the distance is elevated to a parameter alfa determined experimentally.

![img](http://ieeexplore.ieee.org/mediastore/IEEE/content/media/6916489/6927672/6927707/6927707-table-4-small.gif)

The average alfa is 3.29.

### LoRa Propagation Experiments Findings

According to [5]:

* __Foliage__: the attenuation caused by thick foliage is substantial in LoRa and reduces the range of an order of magnitude (from approx. 500m to 50-90m using BW 500MHz and SF 6)
* __Transmission Power:__ not important in terms of range
* __Bandwidth__: from 500 MHz to 125 MHz the range almost double
* __SF__: from 6 to 8 almost double the range
* __Temperature__: at 36° the range drops by half with respect to measurements performed in scenario described in Foliage
* __Consistency of Received signal__: in forest environment almost the same in all directions

## Average Trees height

The average trees height is important in order to determine the altitude at which the drones should fly. To make the problem tractable, only forests in Portugal mainland are considered.
The [prevalent trees](http://ypef.eu/flora_fauna_p) are:

* Maritime pine (Pinus pinaster) - heigth up to 40m
* Eucalyptus (Eucalyptus globulus) -height up to 55m
* Cork oak (Quercus suber) - height up to 20m
* Holm oak (Quercus rotundifolia) - height up to 20m

Given the these measurements, the drones should fly at least at 60 m above the ground to avoid collisions.

## Size of simulated area

Citing [3]:

>If we observe the information regarding large forest fires in the last 30 years, we will see that the most frequent LFFs were those which burned areas between 100 and 500 ha, representing on average 77.9% of the total number of LFFs, and were responsible for 40.3% of the area burned by LFFs during that period. For LFFs over 500 ha, the most frequent ones were those which burned areas between 1 000 and 5 000 ha, representing 8.7% of the total, and were responsible for 30.9% of the area burned by LFFs (Table 2). 

Given the above information, we can assume two different scenarios. A more significant one with a 500ha extension (5 Km^2^) and a more extreme one with 3000-5000 ha (30 - 50 Km^2^). Initially only the 500ha scenario will be considered. 

## References

[1] Savage, N., D. Ndzi, A. Seville, E. Vilar, and J. Austin (2003), Radio wave propagation through vegetation: Factors influencing signal attenuation, Radio Sci., 38, 1088, doi:10.1029/2002RS002758, 5.7

[2]  Anastassiu HT, Vougioukas S, Fronimos T, et al. A Computational Model for Path Loss in Wireless Sensor Networks in Orchard Environments. Sensors (Basel, Switzerland). 2014;14(3):5118-5135. doi:10.3390/s140305118.

[[3]](http://journals.openedition.org/mediterranee/6863) Leite, Flora & Lourenço, Luciano & Bento-Gonçalves, António. (2013). Large forest fires in mainland Portugal, brief characterization. Méditerranée. 121. 53-66. 10.4000/mediterranee.6863.

[[4]](http://ieeexplore.ieee.org/document/6927707/) S. Jiang, C. Portillo-Quintero, A. Sanchez-Azofeifa and M. H. MacGregor, "Predicting RF path loss in forests using satellite measurements of vegetation indices," 39th Annual IEEE Conference on Local Computer Networks Workshops, Edmonton, AB, 2014, pp. 592-596.

\[[5](https://pdfs.semanticscholar.org/9027/ad3f2b4aba423fb8a1d095a4e3d9c0886dd2.pdf)\] LoRa from the City to the Mountains: Exploration of Hardware and Environmental Factors