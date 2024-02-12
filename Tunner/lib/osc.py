import math as mt
import random as rd
from enum import Enum

class Operations(Enum):
	Seno=0
	Cuadratica=1
	Triangular=2
	SierraAna=3
	SierraOpt=4
	Ruido=5
	Segmento=6
	SenoArmonico=7
	Operacion=-1
	
class Oscillador():
	@staticmethod
	def armonic_freq(id:int, scaleID:int = 0) ->float:
		if (scaleID == 0):
			return 8.0 * pow(1.0594630943592952645618252949463, id)
	@staticmethod
	def sin_arm_data(amp:float, id:int, fase:float, org:float, dt:int, zoom:float)->float:
		return amp * mt.sin(Oscillador.armonic_freq(id)* dt * mt.pi/(180 * zoom) + fase) + org
	@staticmethod
	def sin_data(amp:float, freq:float, fase:float, org:float, dt:int, zoom:float)->float:
		return amp * mt.sin(freq * dt * mt.pi/(180 * zoom) + fase) + org
	@staticmethod
	def square_data(amp:float, freq:float, fase:float, org:float, dt:int, zoom:float)->float:
		if(mt.sin(freq * dt * mt.pi/(180 * zoom) + fase) > 0):
			return (amp + org) * zoom
		return (-amp + org) * zoom
	@staticmethod
	def triangle_data(amp:float, freq:float, fase:float, org:float, dt:int, zoom:float)->float:
		return amp * mt.asin(mt.sin(freq * dt * mt.pi/(180* zoom) + fase)) * (2.0/mt.pi) + org
	@staticmethod
	def saw_ana_data(amp:float, freq:float, fase:float, org:float, dt:int, zoom:float, fid:int)->float:
		sub_res = 0
		for j in range(1, int(fid) + 1, 1):
			sub_res += mt.sin(j * freq * dt * mt.pi/(180* zoom)) / j
		return sub_res
	@staticmethod
	def saw_opt_data(amp:float, freq:float, fase:float, org:float, dt:int, zoom:float) ->float:
		if(freq != 0.0):
			return amp * (2.0/mt.pi) * (freq * mt.pi * mt.fmod(dt, zoom/freq) - (mt.pi/2.0) + fase) + org
	@staticmethod
	def noise_data(min_v:float, max_v:float, org:float, zoom:float):
		return rd.uniform(min_v, max_v) * zoom + org
	@staticmethod
	def seg_data(slop:float, org:float, dt:int, zoom:float):
		return (slop * dt + org) * zoom
	@staticmethod
	def op_arrays(a, b, is_add=True):
		for i in range(0,len(a)):
			if(i < len(b)):
				if(is_add):
					a[i] += b[i]
				else:
					a[i] *= b[i]
			else:
				break
	@staticmethod
	def clear_array(a,n=0):
		for i in range(0,len(a)):
			a[i] = n
	@staticmethod
	def array_esc(a,s, is_add = True):
		for i in range(0,len(a)):
			if(is_add):
				a[i] += s
			else:
				a[i] *= s
	@staticmethod
	def operation(instructions, begin:int, sim:int, zoom:float):
		osc = Oscillador()
		data = []
		data.extend(range(0, sim + 1))
		osc.clear_array(data)
		for t in range(0, sim + 1):
			dt = t + begin
			for i in instructions:
				if(i[0] == 0):
					data[t] += osc.sin_data(i[1], i[2], i[3], i[4], dt, zoom)
				elif(i[0] == 1):
					data[t] += osc.square_data(i[1], i[2], i[3], i[4], dt, zoom)
				elif(i[0] == 2):
					data[t] += osc.triangle_data(i[1], i[2], i[3], i[4], dt, zoom)
				elif(i[0] == 3):
					data[t] += osc.saw_ana_data(i[1], i[2], i[3], i[4], dt, zoom, i[5])
				elif(i[0] == 4):
					data[t] += osc.saw_opt_data(i[1], i[2], i[3], i[4], dt, zoom)
				elif(i[0] == 5):
					data[t] += osc.noise_data(i[1], i[2], i[3], zoom)
				elif(i[0] == 6):
					if(t >= i[3] and t <= i[4]):
						data[t] += osc.seg_data(i[1], i[2], dt, zoom)
				elif(i[0] == 7):
					data[t] += osc.sin_arm_data(i[1], int(i[2]), i[3], i[4], dt, zoom)
		return data