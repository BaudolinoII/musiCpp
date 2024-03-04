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
		self.rate_sample = 44100
	def set_rate_sample(self, rs = 44100):
		self.rate_sample = rs
	def set_parameters(self, A:float=-1.0, D:float=-1.0, S:float=-1.0, R:float=-1.0, MV:float=-1.0, SV:float=-1.0, RV:float=-1.0):
		if(A > 0.0):
			self.Attack = A
		if(D > 0.0):
			self.Decay = D
		if(S > 0.0):
			self.Sustain = S
		if(R > 0.0):
			self.Release = R
		if(MV >= 0.0):
			self.Max_Value = MV
		if(SV >= 0.0):
			self.Sus_Value = SV
		if(RV >= 0.0):
			self.Rel_Value = RV
	def get_template(self, x_sim:int = 44100):		
		values = []

		Attack_samp =  int(self.Attack * self.rate_sample)
		Decay_samp =  int(self.Decay * self.rate_sample)
		Sustain_samp =  int(self.Sustain * self.rate_sample)
		Release_samp = int(self.Release * self.rate_sample)

		for i in range(0, x_sim):
			if i < Attack_samp: #Attack
				pend = self.Max_Value / Attack_samp
				org = 0
			elif i < (Decay_samp + Attack_samp): #Decay
				pend = (self.Sus_Value - self.Max_Value) / Decay_samp
				org = self.Max_Value - pend * Attack_samp 
			elif i  < (Sustain_samp + Decay_samp + Attack_samp): #Sustain
				pend = (self.Rel_Value - self.Sus_Value) / Sustain_samp
				org = self.Sus_Value - pend * (Decay_samp + Attack_samp)
			else:#Release
				pend = -self.Rel_Value / Release_samp
				org =  self.Rel_Value - pend * (Sustain_samp + Decay_samp + Attack_samp)

			if(i * pend + org) > 0.0:
				values.append(i * pend + org)
			else:
				values.append(0.0)

		return values
	