import wave
import matplotlib.pyplot as plt
import numpy as np

from .util_math import Util as ut

class AudioFile():
	def __init__(self):
		self.sample_freq = 0
		self.n_samples = 0
		self.signal_wave = 0
		self.duration_time = 0
		self.data = []
		self.sample_time = 0

	def load_archive(self, path):
		self.archive = wave.open(path,'rb')
		self.sample_freq = self.archive.getframerate()
		self.n_samples = self.archive.getnframes()
		self.signal_wave = self.archive.readframes(-1)#Archivo
		self.archive.close()
		self.duration_time = self.n_samples/self.sample_freq
		self.data = np.frombuffer(self.signal_wave, dtype=np.int16)
		self.sample_time = np.linspace(0, self.duration_time, num=self.n_samples * 2)

	def get_slice_at_time(self, begin:float=0.0, time:float=-1.0):
		if(begin < 0.0):
			begin = 0.0
		if(time <= 0.0 or (begin + time) > self.duration_time):
			time = self.duration_time - begin
		begin_samp = ut.round_i(self.sample_freq * begin)
		data_sam = []
		for i in range(0, ut.round_i(self.sample_freq * time)):
			data_sam.append(self.data[i + begin_samp])
		return data_sam
	def get_slice_at_samp(self, begin:int = 0, lenght:int = -1):
		if(begin < 0):
			begin = 0
		if(lenght <= 0 or (begin + lenght) > self.n_samples):
			lenght = self.n_samples - begin
		data_sam = []
		for i in range(0, lenght):
			data_sam.append(self.data[i + begin])
		return data_sam

	def get_duration_time(self):
		return self.duration_time

	def graphic_plot(self):
		plt.figure(figsize=(15,5))
		plt.plot(self.sample_time, self.data)
		plt.title("Señal de Audio")
		plt.ylabel("Señal de Onda")
		plt.xlabel("Tiempo[s]")
		plt.xlim(0,self.duration_time)
		plt.show()