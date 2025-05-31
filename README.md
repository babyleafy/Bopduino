# Bopduino
A re-imagined twist on the classic speed task game Bop It, built using the Arduino UNO R4 and various external devices, coded in C.

## Features
### Tasks
The game starts automatically on power on, and randomly generates task prompts on the LCD screen for the player to complete within a set time limit. Our Boduino game supports four prompts:
1. **Press It!**: Awards points if the button was pressed.
2. **Twist It!**: Awards points if the rotary encoder was rotated from its original position.
3. **Tilt It!**: Awards points if the MPU6500 was tilted at least 60 degrees from its original position.
4. **Scan It!**: Awards points if the RFID tag was scanned by the RFID reader.

### Rules
- If the player completes the task they are awarded 10 points to the running score, which is maintained in memory internally. Additionally, another task is generated and displayed after a short, pre-determined cooldown.
- If the player fails to complete the task in the time, they lose the game and are able to see their accumulated score.
- If the player completes the wrong task (i.e. any task that is not the prompt), they also lose the game.

