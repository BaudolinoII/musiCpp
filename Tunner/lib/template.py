import matplotlib.pyplot as plt

class ADSR():
	def __init__(self):
		self.Attack = 0.2
		self.Decay = 0.2
		self.Sustain = 0.4
		self.Max_Value = 1.0
		self.Sus_Value = 0.6
		self.Rel_Value = 0.4
		self.MinLTime = 1.0
		self.MaxLTime = 3.0
	def set_parameters(self, A:float=-1.0, D:float=-1.0, S:float=-1.0, MV:float=-1.0, SV:float=-1.0, RV:float=-1.0, minT:float=-1.0, maxT:float=-1.0):
		if(A >= 0.0):
			self.Attack = A
		if(D >= 0.0):
			self.Decay = D
		if(S >= 0.0):
			self.Sustain = S
		if(MV >= 0.0):
			self.Max_Value = MV
		if(SV >= 0.0):
			self.Sus_Value = SV
		if(RV >= 0.0):
			self.Rel_Value = RV
		if(minT >= 0.0):
			self.MinLTime = minT
		if(maxT >= 0.0):
			self.MaxLTime = maxT
	def get_template(self, time:float = 1.0, lenght_sam:int = 88200):
		values = []
		if(time < self.MinLTime):
			time = self.MinLTime
		elif(time > self.MaxLTime):
			time = self.MaxLTime

		time_sam = int(time * lenght_sam)
		Attack_samp =  int(self.Attack * time_sam)
		Decay_samp =  int(self.Decay * time_sam)
		Sustain_samp =  int(self.Sustain * time_sam)
		Release_samp = Sustain_samp + Decay_samp + Attack_samp
		#f(x) = mx + b
		#m = (y2 - y1) / (x2 - x1)
		#b = f(r) - mr

		pend = self.Max_Value / Attack_samp #f(x) = mx + b ; m = MV/Attack_samp; b = 0
		for i in range(0, Attack_samp):
			values.append(i * pend) 
		print('Ataque finalizado: n_samples = {}'.format(len(values)))
		pend = (self.Sus_Value - self.Max_Value) / Decay_samp
		org = self.Max_Value - pend * Attack_samp #f(x) = mx + b ; m = (SV - MV)/(Decay_sam); b = MV - m * Attack_samp
		for i in range(Attack_samp, Decay_samp + Attack_samp):
			values.append(i * pend + org)
		print('Caida finalizado: n_samples = {}'.format(len(values)))
		pend = (self.Rel_Value - self.Sus_Value) / Sustain_samp
		org = self.Sus_Value - pend * Decay_samp #f(x) = mx + b ; m = (RV - SV)/(Sustain_sam); b = SV - m * Decay_samp
		for i in range(Decay_samp, Sustain_samp + Decay_samp):
			values.append(i * pend + org)
		print('Sosten finalizado: n_samples = {}'.format(len(values)))

		pend = -self.Rel_Value / (time_sam - Release_samp)
		org = self.Rel_Value - pend * Release_samp #f(x) = mx + b ; m = -RV/(time_sam - Release_sam); b = RV - m * Release_samp
		for i in range(Release_samp, time_sam):
			values.append(i * pend + org)
		print('Liberacion finalizado: n_samples = {}'.format(len(values)))
		return values
	def graphic_plot(self):
		temp = ADSR()
		y_val = temp.get_template(lenght_sam=100)
		x_val = range(0,len(y_val))

		plt.figure(figsize=(15,5))
		plt.plot(x_val, y_val)
		plt.title("Modelo_ADSR")
		plt.ylabel("Amplitud")
		plt.xlabel("Tiempo[s]")
		plt.show()