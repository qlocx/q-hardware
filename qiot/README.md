# Programming QIoT

Make sure your QIoT is connected to a battery pack and j-link.


First time you program a qiot, run command: 

nrfjprog --program ./mfw_nrf9160_1.3.1.zip --verify

Then run command: 

nrfjprog -f nrf91 --program merged_0.98.hex --sectorerase


Done! 