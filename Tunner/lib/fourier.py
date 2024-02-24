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
		self.f_data = self.f_data[0:self.lenght//2] #/ ut.log_10(max(self.f_data))#10000000
		#self.data = self.data / ut.log_10(max(self.data))
		self.freq_x = self.sample_rate * np.arange(0, self.lenght//2) / self.lenght
		self.sam_data = self.data
		self.sam_f_data = self.f_data
		self.sam_freq_x = self.freq_x

	def get_FFT(self, test_data, spectre:int = 1000):
		test_lenght = len(test_data)
		f_test_data = abs(fourier.fft(test_data))
		return f_test_data[0:spectre]

	def set_sample_area_at_time(self,begin:float=0.0, time:float=-1.0, spectre:int=5000):
		if(begin < 0.0):
			begin = 0.0
		if(time <= 0.0 or (begin + time) > self.duration_time):
			time = self.duration_time - begin
		begin_samp = ut.round_i(self.sample_rate * begin)
		lenght_sam = ut.round_i(self.sample_rate * time)

		self.sam_data = self.data[begin_samp:lenght_sam]

		self.sam_lenght = lenght_sam
		self.sam_freq_x = self.sample_rate * np.arange(0, spectre) / lenght_sam
		self.sam_f_data = abs(fourier.fft(self.sam_data))
		self.sam_f_data = self.sam_f_data[0:spectre]
		self.sam_f_data = self.sam_f_data / ut.log_10(max(self.sam_f_data))
		self.sam_data = self.sam_data / ut.log_10(max(self.sam_data))
	def set_sample_area_at_sample(self,begin:int=0, lenght_sam:int=-1, spectre:int=5000):
		if(begin < 0):
			begin = 0
		if(lenght_sam <= 0 or (begin + lenght_sam) > self.lenght):
			lenght_sam = self.lenght - begin
		begin_samp = ut.round_i(self.sample_rate * begin)
		
		self.sam_data = self.data[begin_samp:lenght_sam]
		
		self.sam_lenght = lenght_sam
		self.sam_freq_x = self.sample_rate * np.arange(0, spectre) / lenght_sam
		self.sam_f_data = abs(fourier.fft(self.sam_data))
		self.sam_f_data = self.sam_f_data[0:spectre]
		self.sam_f_data = self.sam_f_data / ut.log_10(max(self.sam_f_data))
		self.sam_data = self.sam_data / ut.log_10(max(self.sam_data))

	def get_amp_sample(self):
		return self.sam_data
	def get_amp_entire(self):
		return self.data / ut.log_10(max(self.data))
	def get_fft_sample(self, begin:float=0.0, time:float=-1.0):
		return self.sam_f_data
	def get_fft_entire(self, begin:int = 0, lenght:int = -1):
		return self.f_data / ut.log_10(max(self.f_data))

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
				armonics.append({float('{:2f}'.format(self.sam_freq_x[pos + begin])):max(aux_data)})
				begin = i - 1
			before = i
		return armonics
	def get_duration_time(self):
		return self.duration_time
	def get_audio_size(self):
		return self.sam_lenght
	def get_rate(self):
		return self.sample_rate

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

f.set_sample_area_at_time(0,1,1000)

arms = f.localize_armonics(flud=0.3)
print('Armonicos localizados en:')
print(arms)

f.graphic_fft_plot()
f.graphic_fft_plot(is_samp = False)
'''