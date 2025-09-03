#!/bin/bash

g++ cam_reader.cpp -o cam_reader `pkg-config --cflags --libs opencv4`
