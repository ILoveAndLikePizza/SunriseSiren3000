# Sunrise Siren 3000
### The most epic NeoPixel based alarm clock
<br>

## Introduction
The Sunrise Siren 3000 is the clock I have in front of my bed. Its job is to display time, temperature and humidity, but most importantly, wake me up whenever I have to wake up. I realize that you may not own a Sunrise Siren 3000 yourself, but I still like to make this project open-source :P  
This repository consists of two relevant parts of the clock:  
- the firmware of the clock, located in the `SunriseSirenFirmware` directory. It is flashed onto the ESP32 inside it by using the [Arduino IDE](https://www.arduino.cc/en/software);
- the control program, **Sunrise Siren Studio**, located in the `SunriseSirenStudio` directory. It is made with the [Gimp ToolKit](https://gtk.org/) (GTK) in C, making it only available to Linux (and WSL2) users. The program has to be compiled from source.

## Why are there no prebuilt binaries?
- The clock's firmware contains sensitive information, which is required to make configuration changes or to flash new firmware. This information is stored in a header file, and the program cannot be compiled without it, therefore obstructing me from providing a prebuilt esptool-ready binary.
- The control program does not work by itself. It also relies on the desktop entry file, icon, and GSettings schema, which are not included in the binary file. Maybe I will provide `deb` and `rpm` packages in the future :)

## Firmware setup
1. Navigate to the `SunriseSirenFirmware` directory.
2. Copy `Authentication.h.example` to `Authentication.h` and change the values in that file to whatever you like.
> [!IMPORTANT]
> Ensure that all your specified values are less than 32 characters. Otherwise, they are very difficult to enter in Sunrise Siren Studio later on.

> [!WARNING]
> Sunrise Siren 3000 passwords are NOT encrypted or hashed anywhere. **DO NOT USE A PASSWORD THAT YOU ALREADY USE SOMEWHERE ELSE!**
3. Launch the Arduino IDE and load the `SunriseSirenFirmware.ino` sketch.
> **Hint:** you can add the firmware sketch to your Arduino IDE sketchbook with a symlink:
> ```bash
> ln -s $(pwd) ~/Arduino
> ```
> Note that:
> - this command assumes you are in the `SunriseSirenFirmware` directory;
> - the location of your sketchbook may vary.
>
> Now, you can load the sketch easier.

4. Ensure that the ESP32 board is installed (if not, check [this tutorial](https://randomnerdtutorials.com/installing-esp32-arduino-ide-2-0/));
5. Ensure that the required libraries are installed:
    - **WiFiManager** *(by tzapu)*;
    - **FastLED** *(by Daniel Garcia)*;
    - **NTPClient** *(by Fabrice Weinberg)*;
    - **SHT2x** *(by Rob Tillaart)*.
6. Go to **Tools** > **Board** and ensure that it is set to "**ESP32 Dev Module**";
7. Click the Upload button.

### Congratulations, the Sunrise Siren 3000 firmware has been flashed successfully!
<br>

## Sunrise Siren Studio installation
1. Ensure that the required libraries are installed.  
With `apt`, those can be installed by running:
    ```bash
    sudo apt install gcc libgtk-3-dev libcurl4-openssl-dev libjson-c-dev
    ```
    And with `dnf`:
    ```bash
    sudo dnf install gcc gtk3-devel libcurl-devel json-c-devel
    ```
2. Navigate to the `SunriseSirenStudio` directory.
3. Compile the program by running:
    ```bash
    make
    ```
4.  If you want to install the program right away, skip to the next step. If you want to test the program first (inside the repository), it is necessary to refer to the compiled GSchema by using the `GSETTINGS_SCHEMA_DIR` environment variable when executing the binary:
    ```bash
    GSETTINGS_SCHEMA_DIR=$(pwd)/schema build/sunrise-siren-studio
    ```
> [!NOTE]
> - This command assumes that you are in the `SunriseSirenStudio` directory.
> - The icon on the connection window may not be visible if the program (and so, its icon) is not installed yet.
5. To install the program on your computer, run:
    ```bash
    sudo make install
    ```
6. To launch the program, simply search for "Sunrise Siren Studio" in the search bar of your computer!

### Congratulations, Sunrise Siren Studio is up and running now!
<br>

## Sunrise Siren Studio uninstallation
1. Simply run:
    ```bash
    sudo make uninstall
    ```
2. Optionally, you can remove all minified and compiled files from the repository:
    ```bash
    make clean
    ```
