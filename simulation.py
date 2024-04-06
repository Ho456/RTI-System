import numpy as np
import oct2py

nodenum = 40
size = nodenum*(nodenum - 1)
oct2py.octave.addpath('C:/Users/tszho/codeclone/Codes')
oct2py.octave.addpath('C:/Users/tszho/codeclone/Codes/TV_reg/Solver')
oct2py.octave.addpath('C:/Users/tszho/codeclone/Codes/TV_reg/Fast_Walsh_Hadamard_Transform')
oct2py.octave.addpath('C:/Users/tszho/codeclone/Codes/TV_reg/Utilities')

print('start loading kernel')
weights = oct2py.octave.test()
print('kernel loaded')
axis1 = 60
axis2 = 60

        
arr = oct2py.octave.ptotload()
print('Ptot loaded')
arravg = oct2py.octave.pincload()
print('Pinc loaded')
o = oct2py.octave.recon(weights, axis1, arravg, arr)

