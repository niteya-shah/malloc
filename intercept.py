import numpy as np
import ctypes
import os
so = ctypes.CDLL('./intercept.so')
so.start_capture.argtypes = [ctypes.c_char_p]
so.stop_capture.argtypes = []
so.start_capture(b'/home/touchdown/work/c++/malloc/testing/')
x = np.random.rand(1000, 1000) @ np.random.rand(1000, 1000) 
so.stop_capture()