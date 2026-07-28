---
# yaml-language-server: $schema=schemas\page.schema.json
Object type:
    - Page
Backlinks:
    - navigation.md
Creation date: "2026-07-23T23:19:56Z"
Created by:
    - camelNote
id: bafyreifuirgze4p7smavg6dgoylcqfyvplg33ieahgrukcb6yzbd2cu2pm
---
# Master pinout   
| STM32 |   Device |                                                     Purpose |
|:------|:---------|:------------------------------------------------------------|
|   GND | TFT\_GND |                                                         GND |
| +3.3V | TFT\_VCC |                                                         VCC |
|    B1 | TFT\_RST |                                                   TFT Reset |
|    B0 |  TFT\_DC |                                                             |
|    A7 |  TFT\_SD |                                                        MOSI |
|    A5 | TFT\_SCL |                                                  SPI1 Clock |
|    A2 |  TFT\_BL | Blacklight - **Active LOW, can also be dimmed through PWM** |
|    A1 |  TFT\_CS |                                             TFT chip select |
|   B15 | SD\_MOSI |                                                SD MOSI SPI2 |
|   B14 | SD\_MISO |                                               SD MISO, SPI2 |
|   B13 |  SD\_CLK |                                                SD CLK, SPI2 |
|    A8 |   SD\_CS |                                              SD chip select |
|       |          |                                                             |

External   
| SD card | Purpose |
|:--------|:--------|
|     VCC |     VCC |
|     GND |     GND |

