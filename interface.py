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
##dat = ['bottom01.xlsx', 'bottom02.xlsx', 'bottom03.xlsx', 'bottom04.xlsx', 'bottom05.xlsx', 'bottom06.xlsx', 'middle01.xlsx', 'middle02.xlsx', 'middle03.xlsx', 'middle04.xlsx', 'middle05.xlsx', 'middle06.xlsx', 'right01.xlsx', 'right02.xlsx', 'right03.xlsx']
##arravg = np.array(pd.read_excel('average.xlsx', header = None)) * -1
##arravg = np.transpose(arravg)
##arravg = np.transpose(arravg)
##arravg*=-1
##a = np.array([[0,4,5,0],[0,5,6,0],[0,6,7,0],[0,7,8,0],[0,8,9,0],[0,9,10,0],[0,10,11,0],[1,4,5,1],[1,5,6,1],[1,6,7,1],[1,7,8,1],[1,8,9,1],[1,9,10,1],[1,10,11,1],[1,11,12,1],[1,12,13,1],[1,13,14,1],[1,14,15,1],[2,4,5,2],[2,5,6,2],[2,6,7,2],[2,7,8,2],[2,8,9,2],[2,9,10,2],[2,10,11,2],[2,11,12,2],[2,12,13,2],[2,13,14,2],[2,14,15,2],[3,4,5,3],[3,5,6,3],[3,6,7,3],[3,7,8,3],[3,8,9,3],[3,9,10,3],[3,10,11,3],[3,11,12,3],[3,12,13,3],[3,13,14,3],[3,14,15,3],[4,8,9,4],[4,9,10,4],[4,10,11,4],[4,11,12,4],[4,12,13,4],[4,13,14,4],[4,14,15,4],[5,8,9,5],[5,9,10,5],[5,10,11,5],[5,11,12,5],[5,12,13,5],[5,13,14,5],[5,14,15,5],[6,8,9,6],[6,9,10,6],[6,10,11,6],[6,11,12,6],[6,12,13,6],[6,13,14,6],[6,14,15,6],[7,8,9,7],[7,9,10,7],[7,10,11,7],[7,11,12,7],[7,12,13,7],[7,13,14,7],[7,14,15,7],[8,12,13,8],[8,13,14,8],[8,14,15,8],[9,12,13,9],[9,13,14,9],[9,14,15,9],[10,12,13,10],[10,13,14,10],[10,14,15,10],[11,12,13,11],[11,13,14,11],[11,14,15,11]])
oct2py.octave.addpath('C:/Users/tszho/sim')
oct2py.octave.addpath('C:/Users/tszho/sim/Solver')
oct2py.octave.addpath('C:/Users/tszho/sim/Fast_Walsh_Hadamard_Transform')
oct2py.octave.addpath('C:/Users/tszho/sim/Utilities')

print('start loading kernel')
weights = oct2py.octave.test()
print('kernel loaded')
axis1 = 50
axis2 = 50

##for filename in dat:
##    print(filename)
##    arr = np.array(pd.read_excel(filename, header=None)) * -1
##    arr = np.transpose(arr)
##    arr = np.transpose(arr)
##    arr*=-1
##    o = oct2py.octave.recon(weights, axis1, arravg, arr)
##    var1 = input('Next?')

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
            
