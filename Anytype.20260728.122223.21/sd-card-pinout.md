---
# yaml-language-server: $schema=schemas\page.schema.json
Object type:
    - Page
Backlinks:
    - navigation.md
Creation date: "2026-07-17T17:29:32Z"
Created by:
    - camelNote
id: bafyreibzkxaneqdfwzk6a6iw67272govs26nbmewyohagfkdplk22o3kru
---
# SD card pinout   
![support_microchip_com_image](files\support_microchip_com_image.jpeg)    
| SD Pin | Native Name |            SPI Function |               Arduino connection |
|:-------|:------------|:------------------------|:---------------------------------|
|      1 |     CD/DAT3 |    **CS** (Chip Select) | Any digital pin (define in code) |
|      2 |         CMD |                **MOSI** |                Hardware MOSI pin |
|      3 |        VSS1 |                     GND |                              GND |
|      4 |         VDD |                **3.3V** |                        3.3V rail |
|      5 |         CLK |                 **SCK** |                 Hardware SCK pin |
|      6 |        VSS2 |                     GND |                              GND |
|      7 |        DAT0 |                **MISO** |                Hardware MISO pin |
|      8 |        DAT1 | NC (unused in SPI mode) |                       leave open |
|      9 |        DAT2 | NC (unused in SPI mode) |                       leave open |

