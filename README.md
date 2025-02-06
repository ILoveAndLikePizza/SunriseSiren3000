# Sunrise Siren 3000
### The most epic NeoPixel based alarm clock
<br>

## Introduction
The Sunrise Siren 3000 is the clock I have in front of my bed. Its job is to display time, temperature and humidity, but most importantly, wake me up whenever I have to wake up. I realize that you may not own a Sunrise Siren 3000 yourself, but I still like to make this project open-source :P  
This repository consists of two relevant parts of the clock:  
- the firmware of the clock, located in the `SunriseSirenFirmware` directory. It is flashed onto the ESP32 inside it by using the [Arduino IDE](https://www.arduino.cc/en/software);
- the control program, **Sunrise Siren Studio**, located in the `SunriseSirenStudio` directory. It is made with the [Gimp ToolKit](https://gtk.org/) (GTK) in C, making it only available to Linux (and WSL2) users. The program has to be compiled from source.

## Firmware setup
1. Navigate to the `SunriseSirenFirmware` directory.
2. Copy `Authentication.h.example` to `Authentication.h` and change the values in that file to whatever you like. (Note that values over 32 characters are very difficult to enter into Sunrise Siren Studio.)
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

Congratulations, the Sunrise Siren 3000 firmware has been flashed successfully!

## Sunrise Siren Studio setup
1. Ensure that the required libraries are installed. On Debian(-based) systems, those can be installed by running:
    ```bash
    sudo apt install libgtk-3-dev libcurl4-openssl-dev libjson-c-dev
    ```
2. Navigate to the `SunriseSirenStudio` directory.
3. Minify the Glade sketch into a UI header file by running:
    ```bash
    ./minify-ui
    ```
4. Compile the program by running:
    ```bash
    make
    ```
5. Optionally, install the program by running:
    ```bash
    sudo make install
    ```
6. Launch the program and follow the instructions given there.

> **Note:** When testing the program inside the repository (without installing it yet), it is necessary to compile and load the correct GSettings by using the `GSETTINGS_SCHEMA_DIR` environment variable:
> ```bash
> make gschema
> GSETTINGS_SCHEMA_DIR=$(pwd)/schema build/sunrise-siren-studio
> ```
> Note that this command assumes you are in the `SunriseSirenStudio` directory.

Congratulations, Sunrise Siren Studio is up and running now!
