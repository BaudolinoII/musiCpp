import math as mt

class Util():
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
		return 10 ** log