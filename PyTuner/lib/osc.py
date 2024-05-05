import math as mt
import random as rd
import numpy as np
import ctypes


class Oscillador():
	@staticmethod
	def sin_data(amp:float, freq:float, dt:float)->float:
		return amp * mt.sin(freq * dt)
	@staticmethod
	def square_data(amp:float, freq:float, dt:float)->float:
		if(mt.sin(freq * dt) > 0):
			return amp
		return -amp
	@staticmethod
	def triangle_data(amp:float, freq:float, dt:float)->float:
		return amp * mt.asin(mt.sin(freq * dt)) * (2.0/mt.pi)
	@staticmethod
	def saw_ana_data(amp:float, freq:float, dt:float, fid:int)->float:
		sub_res = 0
		for j in range(1, int(fid) + 1, 1):
			sub_res += mt.sin(j * freq * dt) / j
		return amp * sub_res
	@staticmethod
	def saw_opt_data(amp:float, freq:float, dt:float) ->float:
		if(freq != 0.0):
			return amp * (2.0/mt.pi) * (freq * mt.pi * mt.fmod(dt, 1/freq) - (mt.pi/2.0))
	@staticmethod
	def noise_data(min_v:float, max_v:float):
		return rd.uniform(min_v, max_v)
	@staticmethod
	def op_arrays(a, b, is_add = True):
		buf = np.arange(0.0, float(len(a)), 1.0)
		for i in range(0,len(a)):
			if(i < len(b)):
				if(is_add):
					buf[i] = a[i] + b[i]
				else:
					buf[i] = a[i] * b[i]
			else:
				buf[i] = a[i]
		return buf
	@staticmethod
	def clear_array(a,n=0):
		for i in range(0,len(a)):
			a[i] = n
	@staticmethod
	def array_esc(a, s, is_add = True):
		for i in range(0,len(a)):
			if(is_add):
				a[i] += s
			else:
				a[i] *= s
	@staticmethod
	def operation(instructions, x_sim:int, sample_rate:int):
		osc = Oscillador()
		data = np.arange(0.0, float(x_sim)) * 0.0
		steps = np.arange(0.0, float(x_sim)) / float(sample_rate)
		index = 0
		for t in steps:
			dt = t * 2.0 * np.pi #FTYPE dFreq = w(dHertz) * dTime 
			for i in instructions:
				if(i[3] != 0.0):# +  dLFOAmplitude * dHertz * (sin(w(dLFOHertz) * dTime));
					dt += i[3] * t * mt.sin(t * i[4]) 
				if(i[0] == 'none'):
					return data
				elif(i[0] == 's'):
					data[index] += osc.sin_data(i[1], i[2], dt)
				elif(i[0] == 'q'):
					data[index] += osc.square_data(i[1], i[2], dt)
				elif(i[0] == 't'):
					data[index] += osc.triangle_data(i[1], i[2], dt)
				elif(i[0] == 'a'):
					data[index] += osc.saw_ana_data(i[1], i[2], dt, i[5])
				elif(i[0] == 'o'):
					data[index] += osc.saw_opt_data(i[1], i[2], dt)
				elif(i[0] == 'n'):
					data[index] += osc.noise_data(i[1], i[2])
			index += 1
		return data
	@staticmethod
	def operation_c(instructions, x_sim:int, sample_rate:int):
		osc_lib = ctypes.cdll.LoadLibrary("./lib/osc.dll")
		
		data = np.arange(0.0, float(x_sim)) * 0.0
		steps = np.arange(0.0, float(x_sim)) / float(sample_rate)
		index = 0
		for t in steps:
			dt = t * 2.0 * np.pi #FTYPE dFreq = w(dHertz) * dTime 
			for i in instructions:
				if(i[0] == 'none'):
					return data
					#FTYPE globalTime, FTYPE ampl, FTYPE dFreq, FTYPE va1, FTYPE va2, FTYPE va3
				elif(i[0] == 's'):
					osc_lib.sine.argtypes = [ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double]
					osc_lib.sine.restype = ctypes.c_double
					data[index] += osc_lib.sine(dt, i[1], i[2], i[3], i[4], i[5])
				elif(i[0] == 'q'):
					osc_lib.square.argtypes = [ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double]
					osc_lib.square.restype = ctypes.c_double
					data[index] += osc_lib.square(dt, i[1], i[2], i[3], i[4], i[5])
				elif(i[0] == 't'):
					osc_lib.traingle.argtypes = [ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double]
					osc_lib.triangle.restype = ctypes.c_double
					data[index] += osc_lib.triangle(dt, i[1], i[2], i[3], i[4], i[5])
				elif(i[0] == 'a'):
					osc_lib.saw.argtypes = [ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double]
					osc_lib.saw.restype = ctypes.c_double
					data[index] += osc_lib.saw(dt, i[1], i[2], i[3], i[4], i[5])
				elif(i[0] == 'n'):
					osc_lib.noise.argtypes = [ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double]
					osc_lib.noise.restype = ctypes.c_double
					data[index] += osc_lib.noise(dt, i[1], i[2], i[3], i[4])
			index += 1
		return data
