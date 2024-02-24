import math as mt
import random as rd
import numpy as np
	
class Oscillador():
	@staticmethod
	def armonic_freq(id:int, scaleID:int = 0) ->float:
		if (scaleID == 0):
			return 8.0 * pow(1.0594630943592952645618252949463, id)
	@staticmethod
	def sin_data(amp:float, freq:float, fase:float, org:float, dt:float)->float:
		return amp * mt.sin(freq * dt + fase) + org
	@staticmethod
	def square_data(amp:float, freq:float, fase:float, org:float, dt:float)->float:
		if(mt.sin(freq * dt + fase) > 0):
			return amp + org
		return -amp + org
	@staticmethod
	def triangle_data(amp:float, freq:float, fase:float, org:float, dt:float)->float:
		return amp * mt.asin(mt.sin(freq * dt + fase)) * (2.0/mt.pi) + org
	@staticmethod
	def saw_ana_data(amp:float, freq:float, fase:float, org:float, dt:float, fid:int)->float:
		sub_res = 0
		for j in range(1, int(fid) + 1, 1):
			sub_res += mt.sin(j * freq * dt) / j
		return sub_res
	@staticmethod
	def saw_opt_data(amp:float, freq:float, fase:float, org:float, dt:float) ->float:
		if(freq != 0.0):
			return amp * (2.0/mt.pi) * (freq * mt.pi * mt.fmod(dt, 1/freq) - (mt.pi/2.0) + fase) + org
	@staticmethod
	def noise_data(min_v:float, max_v:float, org:float):
		return rd.uniform(min_v, max_v) + org
	@staticmethod
	def op_arrays(a, b, is_add = True):
		buf = []
		for i in range(0,len(a)):
			if(i < len(b)):
				if(is_add):
					buf.append(a[i] + b[i])
				else:
					buf.append(a[i] * b[i])
			else:
				return buf
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
	def operation(instructions, sim:int, step:float):
		osc = Oscillador()
		data = []
		data.extend(range(0,sim))
		osc.clear_array(data)
		for t in range(0, sim):
			dt = t * step
			#print('{:.2f}'.format(dt))
			for i in instructions:
				if(i[0] == 'none'):
					return data
				elif(i[0] == 's'):
					data[t] += osc.sin_data(i[1], i[2], i[3], i[4], dt)
				elif(i[0] == 'q'):
					data[t] += osc.square_data(i[1], i[2], i[3], i[4], dt)
				elif(i[0] == 't'):
					data[t] += osc.triangle_data(i[1], i[2], i[3], i[4], dt)
				elif(i[0] == 'a'):
					data[t] += osc.saw_ana_data(i[1], i[2], i[3], i[4], dt, i[5])
				elif(i[0] == 'o'):
					data[t] += osc.saw_opt_data(i[1], i[2], i[3], i[4], dt)
				elif(i[0] == 'n'):
					data[t] += osc.noise_data(i[1], i[2], i[3])

		return data