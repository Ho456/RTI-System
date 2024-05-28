# interface.py
A python file which enables data parsing and serial communication between PC and the base node.
# node_base/main/main.cpp
A C++ file for the base node(ESP32) to acquire data(RSSI) from the measuring nodes and communicate with PC.
# node_measuring/main/main.c
A C file for the measuring nodes(ESP32) to collect data(RSSI) of each link and communicate with the base node.
# weightcal.m
A MATLAB script file which calculates the weight matrix based on the xPRA-LM model.
# recon.m
A MATLAB script file which uses the weight matrix calculated by weightcal.m to run the image reconstruction algorithm(TVAL3).
# simulation.py
A python file for simulation with forward simulation RSSI data.
# demo_video
Video Demonstration for the system.
