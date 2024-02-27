import math as mt

class Util():
	@staticmethod
	def armonic_freq(id:int, scaleID:int = 0) ->float:
		if (scaleID == 0):
			return 8.0 * pow(1.0594630943592952645618252949463, id)
	@staticmethod
	def find_id_note(freq:float)->int:
		return round_i(12.0 * mt.log(freq, 2)) % 12
	@staticmethod
	def inv_disc(a:int,b:int)->int:
		if(a > 0):
			return a % b
		while (a <= 0):
			a += b
		return a
	@staticmethod
	def surround_i(a:int,b:int)->int:
		if(a >= 0):
			return a % b
		while (a < 0):
			a += b
		return a
	@staticmethod
	def surround_f(a:float,b:float)->float:
		while(a >= 0.0):
			a -= b
		while (a < 0):
			a += b
		return a
	@staticmethod
	def round_f(n:float, dec:int = 0)->float:
		expN = n * 10 ** dec
		if abs(expN) - abs(mt.floor(expN)) < 0.5:
			return mt.floor(expN) / 10 ** dec
		return mt.ceil(expN) / 10 ** dec
	@staticmethod
	def round_i(n:float)->int:
		if abs(n) - abs(mt.floor(n)) < 0.5:
			return mt.floor(n)
		return mt.ceil(n)
	@staticmethod
	def log_10(n:float)->int:
		log = 0
		while(n > 10 ** log):
			log += 1
		return 10 ** (log - 1)
	@staticmethod
	def media(n, begin:int, size:int)->float:
		s = 0.0
		for i in range(begin, begin + size):
			if(i < len(n)):
				s += n[i]
			elif(len(n) - begin):
				return s / (len(n) - begin)
			else:
				return 0
		return s / size
	@staticmethod
	def shrink(n, size:int, normal:float = 1.0):
		if(len(n) <= size):
			return n
		data = []
		ut = Util()
		factor =  len(n) / size
		for i in range(0, size):#Muestra Gráfica
			data.append(ut.media(n, ut.round_i(i * factor), ut.round_i(factor)) * normal)
		return data

'''
ut = Util()
data = [1,2,3,4,5,6,7,8,9,10]
sh_data = ut.shrink(n=data, size=2)
print(data)
print(sh_data)
'''