# ESP-32 setup for Microworlds integration

To be able to flash embedded systems with the code contained in this project PlatformIO is needed. You can install it as a VSCode Extension by clicking [here](https://platformio.org/install/ide?install=vscode).

## Setup instructions

Clone the repository on your system and checkout to the "pellonara-esp32" branch.

You will now need to edit a couple of lines in the `main.cpp` file:
- Line 75: Update with the name and password of the WLAN you intend to use the framework in.
- Line 77: Update with the static IP address of the message broker.

If you have multiple embedded systems to flash, ensure each one has a unique device number. Update line 11 of the `main.cpp` file after successfully flashing each embedded system. It's highly recommended to physically write down the assigned device number on each embedded system.

Flash the embedded systems one at a time using either the PlatformIO CLI or VSCode. If you are using VSCode you can either use the bottom bar buttons or search for the "upload" command on the command palette. If you are using the PlatformIO CLI run the following command in the terminal:
```
pio run --target upload
```

After flashing each embedded system, monitor the serial port to ensure that the embedded system is connected to the message broker. Be aware that it might take up to 3 minutes for ESP-32s to connect to the message broker. You can also verify the connection by checking the message broker's terminal output or visiting the overview website. 
