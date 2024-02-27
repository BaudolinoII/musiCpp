import numpy as np
import scipy.fftpack as fourier
import matplotlib.pyplot as plt
import scipy.io.wavfile as waves
import winsound

from .util_math import Util as ut

class Fourier():
	def __init__(self):
		self.lenght = 0
		self.sam_lenght = 0
		self.duration_time = 0.0
		self.org_volume = 0
		self.fundamental = 0
		self.path = ''
		self.data = []
		self.f_data = []
		self.freq_x = []
		self.sam_data = []
		self.sam_f_data = []
		self.sam_freq_x = []

	def load_archive(self, path):
		self.path = path
		self.sample_rate, aux_data = waves.read(self.path)
		self.data = aux_data[:,0]
		self.lenght = len(self.data)
		self.sam_lenght = self.lenght
		self.duration_time = self.lenght / self.sample_rate
		self.f_data = abs(fourier.fft(self.data))
		self.f_data = self.f_data[0:self.lenght//2] 
		self.org_volume = max(self.data)
		self.fundamental = max(self.f_data)
		self.freq_x = self.sample_rate * np.arange(0, self.lenght//2) / self.lenght
		self.sam_data = self.data
		self.sam_f_data = self.f_data
		self.sam_freq_x = self.freq_x
	def get_FFT(self, test_data, bs:int = 0, spectre:int = 50000):
		test_lenght = len(test_data)
		f_test_data = abs(fourier.fft(test_data))
		return f_test_data[bs:spectre + bs]

	def set_sample_area_at_time(self, begin:float=0.0, time:float=-1.0, bs:int = 0, spectre:int=5000):
		if(begin < 0.0):#Seguro de no tener inicios negativos
			begin = 0.0
		if(time <= 0.0):#La duración de tiempo debe ser superior a 0
			time = self.duration_time
		excess = 0
		if((begin + time) > self.duration_time): 
			excess = begin + time - self.duration_time #El excedente debe considerarse para completar la muestra
			time = self.duration_time - begin #En caso de exigir más, solo otorga lo que puede
			
		excess_sam = ut.round_i(self.sample_rate * excess)
		begin_samp = ut.round_i(self.sample_rate * begin)
		lenght_sam = ut.round_i(self.sample_rate * time)

		self.sam_data = self.data[begin_samp:lenght_sam + begin_samp]
		self.sam_lenght = lenght_sam

		if(bs < 0):
			bs = 0
		if((spectre <= 0) and ((bs + spectre) > self.sam_lenght//2)):
			spectre = self.sam_lenght//2 - bs
		if((spectre + begin) > 50000):
			spectre = 50000 - bs
		
		self.sam_freq_x = self.sample_rate * np.arange(bs, spectre + bs) / lenght_sam
		self.sam_f_data = abs(fourier.fft(self.sam_data))
		self.sam_f_data = self.sam_f_data[bs:spectre + bs]
		self.org_volume = max(self.sam_data)
		self.fundamental = max(self.sam_f_data)
		self.sam_f_data = self.sam_f_data / self.fundamental
		self.sam_data = self.sam_data / self.org_volume
		if excess_sam:#si existe exceso
			self.sam_data = np.append(self.sam_data, np.arange(0,excess_sam) * 0.0)
	def set_sample_area_at_sample(self,begin:int=0, lenght_sam:int=-1, bs:int = 0, spectre:int=50000):
		if(begin < 0):
			begin = 0
		if(lenght_sam <= 0):
			lenght_sam = self.lenght
		excess = 0
		if((begin + lenght_sam) > self.lenght):
			excess = begin + lenght_sam - self.lenght
			lenght_sam = self.lenght - begin
		
		self.sam_data = self.data[begin:lenght_sam + begin]
		self.sam_lenght = lenght_sam

		if(bs < 0):
			bs = 0
		if((spectre <= 0) or ((bs + spectre) > self.sam_lenght//2)):
			spectre = self.sam_lenght//2 - bs
		if((spectre + begin) > 50000):
			spectre = 50000 - bs

		self.sam_freq_x = self.sample_rate * np.arange(bs, spectre + bs) / lenght_sam
		self.sam_f_data = abs(fourier.fft(self.sam_data))
		self.sam_f_data = self.sam_f_data[bs:spectre + bs]
		self.org_volume = max(self.sam_data)
		self.fundamental = max(self.sam_f_data)
		self.sam_f_data = self.sam_f_data / self.fundamental
		self.sam_data = self.sam_data / self.org_volume
		if excess:#si existe exceso
			self.sam_data = np.append(self.sam_data, np.arange(0,excess) * 0.0)

	def get_amp_sample(self):
		return self.sam_data
	def get_amp_entire(self):
		return self.data / max(self.data)
	def get_fft_sample(self, begin:float=0.0, time:float=-1.0):
		return self.sam_f_data
	def get_fft_entire(self, begin:int = 0, lenght:int = -1):
		return self.f_data / max(self.f_data)

	def localize_fundamental(self):
		pos = np.where(self.sam_f_data == max(self.sam_f_data))[0][0]
		fund = self.sam_freq_x[pos]
		return float('{:.2f}'.format(fund))
	def localize_armonics(self, flud = 0.25):
		armonics = []
		aux_data = []
		positions = np.where(self.sam_f_data >= flud)[0]
		begin = positions[0] - 1
		before = positions[0]
		for i in positions:
			if(i - before > 1):
				aux_data = self.sam_f_data[begin: before + 1]
				pos = np.where(aux_data == max(aux_data))[0][0]
				armonics.append( (float('{:.2f}'.format(self.sam_freq_x[pos + begin])) , float('{:.2f}'.format(max(aux_data)))) )
				begin = i - 1
			before = i
		return armonics

	def get_duration_time(self):
		return self.duration_time
	def get_audio_size(self):
		return self.sam_lenght
	def get_rate(self):
		return self.sample_rate
	def get_lenght_spectre(self):
		if(self.sam_lenght//2 < 50000):
			return self.sam_lenght//2
		return 50000

	def graphic_fft_plot(self, is_samp = True):
		plt.figure(figsize=(15,5))
		if is_samp:
			plt.plot(self.sam_freq_x, self.sam_f_data)
		else:
			plt.plot(self.freq_x, self.f_data)
		plt.xlabel('Frecuencia (Hz)', fontsize='14')
		plt.ylabel('Amplitud FFT', fontsize='14')
		plt.show()
	def graphic_amp_plot(self, is_samp = True):
		plt.figure(figsize=(15,5))

		if is_samp:
			n = np.arange(0,self.sam_lenght) / self.sample_rate
			plt.plot(n, self.sam_data)
		else:
			n = np.arange(0,self.lenght) / self.sample_rate
			plt.plot(n, self.data)

		plt.xlabel('Tiempo[s]', fontsize='14')
		plt.ylabel('Señal de Onda', fontsize='14')
		plt.xlim(0,self.duration_time)
		plt.show()

'''
f = Fourier()
f.load_archive(path='../samples/guitarra_sample.wav')

f.set_sample_area_at_time(0.0, 1.0, 1000)

print('Armonicos localizados en:')
print(f.localize_armonics(flud=0.2))
print('Fundamental:')
print(f.localize_fundamental())

f.graphic_fft_plot()
#f.graphic_fft_plot(is_samp = False)
'''