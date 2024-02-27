import matplotlib.pyplot as plt
import numpy as np

class ADSR():
	def __init__(self):
		self.Attack = 0.2
		self.Decay = 0.2
		self.Sustain = 0.4
		self.Release = 0.2
		self.Max_Value = 1.0
		self.Sus_Value = 0.6
		self.Rel_Value = 0.4
		self.lim = 44100
		self.size = 44100

	def set_parameters(self, A:float=-1.0, D:float=-1.0, S:float=-1.0, R:float=-1.0, MV:float=-1.0, SV:float=-1.0, RV:float=-1.0):
		if(A >= 0.0):
			self.Attack = A
		if(D >= 0.0):
			self.Decay = D
		if(S >= 0.0):
			self.Sustain = S
		if(R >= 0.0):
			self.Release = R
		if(MV >= 0.0):
			self.Max_Value = MV
		if(SV >= 0.0):
			self.Sus_Value = SV
		if(RV >= 0.0):
			self.Rel_Value = RV
	def get_template(self, lim:int = 44100, x_sim:int = 44100, duration:float = 1.0):
		self.size = x_sim
		if(lim <= self.size):
			self.lim = lim
		else:
			self.lim = self.size
		
		values = []

		Attack_samp =  int(self.Attack * self.lim * duration)
		Decay_samp =  int(self.Decay * self.lim * duration)
		Sustain_samp =  int(self.Sustain * self.lim * duration)
		Release_samp = int(self.Release * self.lim * duration)

		pend = self.Max_Value / Attack_samp
		for i in range(0, Attack_samp):
			values.append(i * pend) 

		pend = (self.Sus_Value - self.Max_Value) / Decay_samp
		org = self.Max_Value - pend * Attack_samp 
		for i in range(Attack_samp, Decay_samp + Attack_samp):
			values.append(i * pend + org)

		pend = (self.Rel_Value - self.Sus_Value) / Sustain_samp
		org = self.Sus_Value - pend * Decay_samp
		for i in range(Decay_samp, Sustain_samp + Decay_samp):
			values.append(i * pend + org)

		pend = -self.Rel_Value / Release_samp
		org =  self.Rel_Value - pend * Sustain_samp
		for i in range(Sustain_samp, Release_samp + Sustain_samp):
			values.append(i * pend + org)

		rest = Release_samp + Sustain_samp + Decay_samp + Attack_samp
		if(rest < x_sim):
			for i in range(rest, self.size):
				values.append(0)

		return values
	def graphic_plot(self):
		temp = ADSR()
		y_val = temp.get_template(self.lim, self.size)
		#print(len(y_val))
		x_val = np.arange(0,len(y_val)) / len(y_val)

		plt.figure(figsize=(15,5))
		plt.plot(x_val, y_val)
		plt.title("Modelo_ADSR")
		plt.ylabel("Amplitud")
		plt.xlabel("Tiempo[s]")
		plt.show()
'''
temp = ADSR()
tmp = temp.get_template(44100, 50000)
temp.graphic_plot()
'''