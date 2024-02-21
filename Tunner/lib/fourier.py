import numpy as np
import scipy.fftpack as fourier
import matplotlib.pyplot as plt
import scipy.io.wavfile as waves
import winsound

from .util_math import Util as ut

class Fourier():
	def __init__(self):
		self.samples = 0
		self.data = []
		self.f_data = []
		self.lenght = 0
		self.duration_time = 0.0
		self.path = ''

	def load_archive(self, path):
		self.path = path
		self.sample_rate, aux_data = waves.read(self.path)
		self.data = aux_data[:,0]
		self.lenght = len(self.data)
		self.duration_time = self.lenght / self.sample_rate
		self.f_data = abs(fourier.fft(self.data))
		self.f_data = self.f_data[0:self.lenght//2] / ut.log_10(max(self.f_data))#10000000
		self.data = self.data / ut.log_10(max(self.data))
		self.freq_x = self.sample_rate * np.arange(0, self.lenght//2) / self.lenght

	def get_amp_slice_at_time(self, begin:float=0.0, time:float=-1.0):
		if(begin < 0.0):
			begin = 0.0
		if(time <= 0.0 or (begin + time) > self.duration_time):
			time = self.duration_time - begin
		begin_samp = ut.round_i(self.sample_rate * begin)
		data_sam = []
		for i in range(0, ut.round_i(self.samples * time)):
			data_sam.append(self.data[i + begin_samp])
		return data_sam
	def get_amp_slice_at_samp(self, begin:int = 0, lenght:int = -1):
		if(begin < 0):
			begin = 0
		if(lenght <= 0 or (begin + lenght) > self.lenght):
			lenght = self.lenght - begin
		data_sam = []
		for i in range(0, lenght):
			data_sam.append(self.data[i + begin])
		return data_sam	
	def get_fft_slice_at_time(self, begin:float=0.0, time:float=-1.0):
		if(begin < 0.0):
			begin = 0.0
		if(time <= 0.0 or (begin + time) > self.duration_time):
			time = self.duration_time - begin
		begin_samp = ut.round_i(self.sample_rate * begin)//2
		data_sam = []
		for i in range(0, ut.round_i(self.samples * time)//2):
			data_sam.append(self.f_data[i + begin_samp])
		return data_sam
	def get_fft_slice_at_samp(self, begin:int = 0, lenght:int = -1):
		if(begin < 0):
			begin = 0
		if(lenght <= 0 or (begin + lenght) > self.lenght // 2):
			lenght = self.lenght//2 - begin
		data_sam = []
		for i in range(0, lenght):
			data_sam.append(self.f_data[i + begin])
		return data_sam

	def localize_armonics(self, flud = 0.25):
		armonics = []
		aux_data = []
		positions = np.where(self.f_data >= flud)[0]
		begin = positions[0] - 1
		before = positions[0]
		for i in positions:
			if(i - before > 1):
				aux_data = self.f_data[begin: before + 1]
				pos = np.where(aux_data == max(aux_data))[0][0]
				armonics.append({float('{:2f}'.format(self.freq_x[pos + begin])):max(aux_data)})
				begin = i - 1
			before = i
		return armonics
	def get_dutation_time(self):
		return self.duration_time

	def graphic_fft_plot(self):
		plt.figure(figsize=(15,5))
		plt.plot(self.freq_x, self.f_data)
		plt.xlabel('Frecuencia (Hz)', fontsize='14')
		plt.ylabel('Amplitud FFT', fontsize='14')
		plt.xlim(0,5000)
		plt.show()
	def graphic_amp_plot(self):
		n = np.arange(0,self.lenght) / self.sample_rate

		plt.figure(figsize=(15,5))
		plt.plot(n, self.data)
		plt.xlabel('Tiempo[s]', fontsize='14')
		plt.ylabel('Señal de Onda', fontsize='14')
		plt.xlim(0,self.duration_time)
		plt.show()

'''
f = Fourier()
f.load_archive(path='../samples/guitarra_sample.wav')

arms = f.localize_armonics(flud=0.3)
print('Armonicos localizados en:')
print(arms)

f.graphic_fft_plot()
f.graphic_amp_plot()
'''