**About AVDD/Excitation voltage**  
I am testing 5V,4.3V LDOs for AVDD to see if the PCB can achieve +/-0.005 usable resolution when plugged in to 5V source.   
When my source voltage is 5V (usb wall charger) I get 4.73V on the AVDD LDO input (don't forget our BAT20J voltage drop). No need to force it to 3.3V (as in the proposed BOM).  
Note that the analog LDO gets its input directly from the P-Channel MOSFET and is not constrained by our digital voltage LDO. We can use 5V LDO for analog excitation voltage and 3.3 for the rest.  
The proposed TLV75533PDBVR for the DVDD we can go up to 5.5V input, so if you wish to increase the input voltage, be careful.  


**Upcoming changes (March 2019)**    
-Trying out touch button solutions because the internal capacitive sensors of ESP32 are not so good behind plastic.  
-I changed (again) the RC filters. X2Y are gone (are expensive, difficult to find and could not measure any difference) and I'm going back to 100% ADS1232 reference design with simple RC plus some more passthrough EMI filters (0805 on this section)  

