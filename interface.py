import numpy as np
import pandas as pd
import math as mt
import os
import matplotlib.pyplot as plt
import oct2py
import serial

li = []
obj = serial.Serial('COM3', 115200)
nodenum = 16
size = nodenum*(nodenum - 1)
scanned = False
num = 0
scansize = 20
scanpixels = np.zeros((scansize, nodenum, nodenum - 1)).astype(int)
oct2py.octave.addpath('C:/Users/tszho/sim')
oct2py.octave.addpath('C:/Users/tszho/sim/Solver')
oct2py.octave.addpath('C:/Users/tszho/sim/Fast_Walsh_Hadamard_Transform')
oct2py.octave.addpath('C:/Users/tszho/sim/Utilities')

print('start loading kernel')
weights = oct2py.octave.test()
print('kernel loaded')
axis1 = 50
axis2 = 50

while True:
    data = obj.read()
    if data == b'|':
        data = obj.read(size)
        for i in range(size):
            li.append(data[i])
        arr = np.array(li)
        arr = arr.reshape((nodenum, nodenum - 1))
        if scanned == False:
            scanpixels[num] = arr
            num+=1
            print('Scanning environment...'+str(num)+'/'+str(scansize))
            if num == scansize:
                arravg = np.mean(scanpixels, axis=0)
                arravg = np.transpose(arravg)
                arravg*=-1
                scanned = True
        else:
            arr = np.transpose(arr)
            arr*=-1
            o = oct2py.octave.recon(weights, axis1, arravg, arr)
        obj.reset_input_buffer()
        li.clear()
            
