---

# v4l2-capture-real-time-image-acquisition

## Project Overview

This project is based on the **V4L2 (Video4Linux2)** framework and aims to achieve real-time image acquisition. By operating camera devices under Linux, it efficiently captures and processes image data, making it suitable for embedded systems and applications requiring real-time vision.

## Features

- Supports real-time camera image acquisition
- Stable and efficient, based on V4L2 API
- Configurable acquisition parameters (resolution, frame rate, etc.)
- Simple and clear code structure
- Easy to integrate into other projects

## Code Structure

- `cam_reader.cpp`: Core acquisition logic, responsible for camera initialization, data acquisition, and resource release
- `Makefile`: Project build script (if available)
- Other auxiliary files

## Environment Dependencies

- Linux system
- V4L2 development library installed
    ```bash
    sudo apt-get install libv4l-dev
    ```
- C++ compiler (e.g., `g++`)

## Build Instructions

In the project root directory, execute:

```bash
g++ -o cam_reader cam_reader.cpp -lv4l2
```

## Run Instructions

Plug in the camera device and run:

```bash
./cam_reader
```

To specify devices or parameters, please set or modify them according to the actual code.

## Application Scenarios

- Real-time image acquisition and processing
- Machine vision, intelligent surveillance
- Embedded Linux device development

## References & Documentation

- [V4L2 Official Documentation](https://linuxtv.org/downloads/v4l-dvb-apis/)
- [cam_reader.cpp Source Code](./cam_reader.cpp)
