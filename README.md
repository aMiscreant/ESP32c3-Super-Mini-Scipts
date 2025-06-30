
# ESP32c3-WiFi-Scripts

This repository contains a collection of Wi-Fi related scripts designed for the ESP32-C3 microcontroller. The scripts cover a variety of functionalities and use cases, including Wi-Fi scanning, packet sniffing, network monitoring, and other wireless operations. These scripts are intended to serve as a starting point or toolkit for anyone interested in exploring the Wi-Fi capabilities of the ESP32-C3, whether for IoT applications, security research, or learning purposes.
Features:

-    Wi-Fi Sniffer: Capture and analyze Wi-Fi networks, extract SSID, MAC addresses, and RSSI values.

-    Wi-Fi Scanner: Scan available networks, list SSIDs, signal strengths, and additional metadata.

-    Network Monitoring: Monitor Wi-Fi traffic and perform basic network diagnostics.

-    Channel Hopping: Automatically cycle through available Wi-Fi channels for more comprehensive scanning.

-    Wi-Fi Configuration: Scripts for setting up and configuring the ESP32-C3 Wi-Fi functionality.

Each script is designed to work with the ESP32-C3’s built-in Wi-Fi features, leveraging the ESP-IDF (Espressif IoT Development Framework) for maximum compatibility and performance.


To get started with idf.py and set up the ESP32-C3 for building a project like a Wi-Fi sniffer, you’ll need to go through a few installation and configuration steps. Here’s a detailed breakdown of what needs to be done:
1. Install ESP-IDF

Before you can use idf.py (ESP-IDF’s command-line tool), you'll need to install the ESP-IDF environment.

a. Clone ESP-IDF repository:
ESP-IDF is the official development framework for the ESP32 series chips, and it's available on GitHub. To clone the repository:

    git clone --recursive https://github.com/espressif/esp-idf.git

b. Set up the ESP-IDF environment:

After cloning, you'll need to install dependencies. First, navigate into the esp-idf directory:

    cd esp-idf

To set up the environment, use the following script:

For Linux/Mac:

    ./install.sh

For Windows (via PowerShell):

    .\install.ps1

The script will install necessary dependencies, including toolchains and libraries.

c. Set up the environment variables:

After installation, you need to set up the ESP-IDF environment by sourcing the export.sh script (Linux/macOS) or export.ps1 (Windows).

For Linux/Mac:

    source export.sh

For Windows (via PowerShell):

    .\export.ps1

This will set environment variables, including the path to the toolchain and other required tools.
2. Install Python Dependencies:

ESP-IDF relies on Python packages. Install the required Python packages using:

    pip install -r requirements.txt

3. Install the Toolchain:

ESP32-C3 uses a RISC-V toolchain for building code. If you're on a platform like Ubuntu, you can use the pre-built toolchain, but on macOS and Windows, you’ll need to follow platform-specific instructions (which should be in the ESP-IDF documentation).

For Ubuntu:

    sudo apt-get install gcc-riscv32-esp32-elf

For macOS:

Use brew (Homebrew package manager):

    brew tap espressif/tools
    brew install riscv-esp32-elf-gcc

4. Set the Target to ESP32-C3:

With idf.py installed and the environment set up, you can start building projects. To target the ESP32-C3, navigate to your project folder, then use idf.py to set the target:

    idf.py set-target esp32c3

This ensures that the build system uses the correct settings for the ESP32-C3.
5. Create a New Project:

Now that the environment is set, you can create a new project (e.g., sniffer).

a. Create a new directory for the project:

    mkdir sniffer
    cd sniffer

b. Set up a template project:

You can either start from scratch or use one of ESP-IDF’s example projects as a template. Let’s use the Wi-Fi sniffer example:

    cp -r $IDF_PATH/examples/wifi/sniffer .
    cd sniffer

Here, $IDF_PATH is the path to your ESP-IDF installation.

c. Configure the project:

Now you need to configure the project. Run:

    idf.py menuconfig

This opens a configuration menu where you can set various options like the chip model, Wi-Fi settings, and others.

Set the Target to ESP32-C3.

    Make sure you have the correct Wi-Fi configurations set.

6. Build the Project:

Once the configuration is set, you can build the project using:

    idf.py build

This will compile the project, including linking and generating the firmware image.
7. Flash the ESP32-C3:

Now, you need to flash the firmware onto the ESP32-C3.

First, connect your ESP32-C3 to your computer via USB. Once it’s connected, check which device port it’s connected to.

Use this command to flash the project:

    idf.py -p /dev/ttyUSB0 flash

Replace /dev/ttyUSB0 with the correct port if it's different on your system.
8. Monitor the Output:

Once the ESP32-C3 is flashed, you can use the idf.py monitor command to see the output from the device:

    idf.py monitor

This will show debug output, including any information from the sniffer code you’ve uploaded.
9. Customizing the Sniffer:

    To customize the sniffer, you can modify the main source code of the sniffer example.

The main code for the sniffer is usually located in main/sniffer.c or similar. Here, you can adjust parameters for packet capture, logging, or add custom processing for the packets.
