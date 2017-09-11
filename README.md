# ModeMCU1.0
NodeMCU repo

# To flash SPIFFS follow below steps
1. Create a data directory where the program is saved.
2. Save files which you want in the SPIFFS system.
3. Make sure filename does not extend 31 characters.
4. To load the SPIFFS,
  - hold FLASH button.
  - push RST button.
  - release FLASH button.
5. Wait till the SPIFFS is loaded.
6. Upload the example which is going to use the SPIFFS files.

# Values for different condition based on actual reading of moister sensor.
1. Water only : 405/ 1023
2. Dry air : 961/ 1023
3. Recently watered soil : 320 - 340 / 1023
4. Warm hands : 900 - 920 / 1023
