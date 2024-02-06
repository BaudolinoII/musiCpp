import math as mt
import random as rd

class Oscillador():
	@staticmethod
	def sin_data(amp:float, freq:float, fase:float, org:float, begin:int, end:int, zoom:float):
		data = []
		for i in range(begin,end,1):
			data.append(amp * mt.sin(freq * i * mt.pi/(360 * zoom) + fase) + org)
		return data
	@staticmethod
	def square_data(amp:float, freq:float, fase:float, org:float, begin:int, end:int, zoom:float):
		data = []
		for i in range(begin,end,1):
			if(mt.sin(freq * i * mt.pi/(360 * zoom) + fase) > 0):
				data.append(amp + org)
			else:
				data.append(-amp + org)
		return data
	@staticmethod
	def triangle_data(amp:float, freq:float, fase:float, org:float, begin:int, end:int, zoom:float):
		data = []
		for i in range(begin,end,1):
			data.append(amp * mt.asin(mt.sin(freq * i * mt.pi/(360* zoom) + fase)) * (2.0/mt.pi) + org)
		return data
	@staticmethod
	def saw_ana_data(amp:float, freq:float, fase:float, org:float, begin:int, end:int, zoom:float, fid:int):
		data = []
		for i in range(begin,end,1):
			sub_res = 0
			for j in range(1,fid + 1,1):
				sub_res += mt.sin(j * freq * i * mt.pi/(360* zoom)) / j
			data.append(amp * sub_res * (2.0/mt.pi + fase) + org)
		return data
	@staticmethod
	def saw_opt_data(amp:float, freq:float, fase:float, org:float, begin:int, end:int, zoom:float):
		data = []
		if(freq != 0.0):
			for i in range(begin,end,1):
				data.append(amp * (2.0/mt.pi) * (freq * mt.pi * mt.fmod(i, zoom/freq) - (mt.pi/2.0) + fase) + org)
		return data
	@staticmethod
	def noise_data(min_v:float, max_v:float, org:float, begin:int, end:int, zoom:float):
		data = []
		for i in range(begin,end,1):
			data.append(rd.uniform(min_v,max_v)*zoom + org)
		return data
	@staticmethod
	def seg_data(slop:float, org:float, begin:int, end:int, zoom:float):
		data = []
		for i in range(begin,end,1):
			data.append((i * slop + org)*zoom)
		return data
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
	def operation(i, output):
		osc = Oscillador()
		if(i[0] == "Seno"):
			osc.op_arrays(output, osc.sin_data(i[1], i[2], i[3], i[4], i[5], i[6], i[7]), True)
		elif(i[0] == "Cuadratica"):
			osc.op_arrays(output, osc.square_data(i[1], i[2], i[3], i[4], i[5], i[6], i[7]), True)
		elif(i[0] == "Triangular"):
			osc.op_arrays(output, osc.triangle_data(i[1], i[2], i[3], i[4], i[5], i[6], i[7]), True)
		elif(i[0] == "SierraAna"):
			osc.op_arrays(output, osc.saw_ana_data(i[1], i[2], i[3], i[4], i[5], i[6], i[7], i[8]), True)
		elif(i[0] == "SierraOpt"):
			osc.op_arrays(output, osc.saw_opt_data(i[1], i[2], i[3], i[4], i[5], i[6], i[7]), True)
		elif(i[0] == "Ruido"):
			osc.op_arrays(output, osc.noise_data(i[1], i[2], i[3], i[5], i[6], i[7]), True)
		elif(i[0] == "Segmento"):
			osc.op_arrays(output, osc.seg_data(i[1], i[2], int(i[3]), int(i[4]), i[7]), True)
	@staticmethod
	def inv_disc(a,b):
		if(a > 0):
			return a % b
		while (a <= 0):
			a += b
		return a
	@staticmethod
	def round(n:float)->int:
		if abs(n) - abs(mt.floor(n)) < 0.5:
			return mt.floor(n)
		return mt.ceil(n)