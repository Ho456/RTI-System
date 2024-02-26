# interface.py
A python file which enables serial communication between PC and the base node.
# node_base.cpp
A C++ file for the base node(ESP32) to acquire data(RSSI) from the measuring nodes and communicate with PC.
# node_measuring.c
A C file for the measuring nodes(ESP32) to collect data(RSSI) of each link and communicate with the base node.
# weightcal.m
A MATLAB script file which calculates the weight matrix based on the xPRA model.
# recon.m:
A MATLAB script file which uses the weight matrix calculated by weightcal.m to run the image reconstruction algorithm(TVAL3).