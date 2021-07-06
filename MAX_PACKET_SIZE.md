# LoRaWAN Max Packet Length
The maximum size of a packet that you can send when using LoRaWAN depends on the selected datarate.

## An overview of packet size by datarate and region.

| Datarate | EU868 | US915 | AU915 | KR920 | AS923 | IN865 | CN470 | EU433 |
| -- | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: |
| DR0 | 51 | 11 | 51 | 51 | 59/ND | 51 | 51 | 51 |
| DR1 | 51 | 53 | 51 | 51 | 59/ND | 51 | 51 | 51 |
| DR2 | 51 | 125 | 51 | 51 | 59/19 | 51 | 51 | 51 |
| DR3 | 115 | 242 | 115 | 115 | 123/61 | 115 | 115 | 115 |
| DR4 | 242 | 242 | 242 | 242 | 250/133 | 242 | 242 | 242 |
| DR5 | 242 | ND | 242 | 242 | 250/250 | 242 | 242 | 242 |
| DR6 | 242 | ND | 242 | ND | 250/250 | 242 | ND | 242 |
| DR7 | 242 | ND | ND | ND | 250/250 | 242 | ND | 242 |
| DR8 | ND | 53 | 53 | ND | ND | ND | ND | ND |
| DR9 | ND | 129 | 129 | ND | ND | ND | ND | ND |
| DR10 | ND | 242 | 242 | ND | ND | ND | ND | ND |
| DR11 | ND | 242 | 242 | ND | ND | ND | ND | ND |
| DR12 | ND | 242 | 242 | ND | ND | ND | ND | ND |
| DR13 | ND | 242 | 242 | ND | ND | ND | ND | ND |
| DR14 | ND | ND | ND | ND | ND | ND | ND | ND |
| DR15 | ND | ND | ND | ND | ND | ND | ND | ND |

**Remarks**      
_`ND`_ Data rate not defined    
For _`AS923`_ first value is DwellTime = disabled, second value is DwellTime enabled    


----

**REMARK**    
> **`CN779-787 devices may not be produced, imported or installed after 2021-01-01; deployed devices may continue to operate through their normal end-of-life.`**    

----
