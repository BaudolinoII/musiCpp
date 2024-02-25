import tkinter as tk
from tkinter import ttk
from tkinter import filedialog
from tkinter.filedialog import askopenfile

import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import numpy as np

from lib.osc import Oscillador as osc
from lib.util_math import Util as ut
from lib.xml_man import XML_Manager
from lib.fourier import Fourier
import lib.template as tp

#color = '#FFFFFF'

class Application(tk.Frame):
	def __init__(self, master):
		super().__init__(master)
		self.master = master

		self.fig, self.ax = plt.subplots()
		self.cv_main = FigureCanvasTkAgg(self.fig, master=self.master)
		self.txb_e = tk.Entry(self.master)
		self.scroll_x = tk.Scrollbar(self.master)
		self.scroll_y = tk.Scrollbar(self.master)
		self.txb_ab = tk.Entry(self.master)
		self.txb_at = tk.Entry(self.master)
		self.btn_a_graf = tk.Button(self.master)
		self.btn_armonic = tk.Button(self.master)

		self.file_inst = '../instrumento.xml'
		self.file_audio = '../sonido.wav'
		self.xman = XML_Manager(self.file_inst)
		self.fou = Fourier()
		self.adsr = tp.ADSR()

		#Valores de simulacion
		self.begin_audio = 0.0
		self.time_audio = 1.0
		self.spectre = 1000
		self.iv = [1.0, 1.0, 0.0, 0.0, 20]
		self.tv = [0.2, 0.2, 0.4, 0.2, 1.0, 0.6, 0.4]
		self.act_meth_val = True
		self.act_temp_val = True
		self.curr_sim = 44100
		self.rate_sim = 44100
		self.scale = (2 * np.pi) / self.rate_sim
		#Buffers		
		self.hot_data = []
		self.cold_data = []
		self.temp_data = []
		self.audio_data = []
		self.f_audio_data = []
		self.f_cold_data = []
		self.f_hot_data = []

		self.create_vars()
		self.create_widgets()
		self.master.geometry("900x860")
		self.master.state('zoomed') # "1920x980"
	#Dibujo de Funciones
	def get_co(self, exp):
		if exp == "Seno":
			return 's'
		if exp == "Cuadratica":
			return 'q'
		if exp == "Triangular":
			return 't'
		if exp == "SierraAna":
			return 'a'
		if exp == "SierraOpt":
			return 'o'
		if exp == "Ruido":
			return 'n'
		if exp == "s":
			return 'Seno'
		if exp == "q":
			return 'Cuadratica'
		if exp == "t":
			return 'Triangular'
		if exp == "a":
			return 'SierraAna'
		if exp == "o":
			return 'SierraOpt'
		if exp == "n":
			return 'Ruido'
		return 'none'
	def update_hot_data(self):
		input_data = [[self.get_co(self.curr_expr.get()), self.iv[0], self.iv[1], self.iv[2], self.iv[3], self.iv[4]]]
		self.hot_data = osc.operation(input_data, self.curr_sim, self.scale)
		self.f_hot_data = self.fou.get_FFT(self.hot_data, self.spectre)
		if max(self.f_hot_data):
			self.f_hot_data = self.f_hot_data * ( self.iv[0] / max(self.f_hot_data))
	def update_cold_data(self):
		method_data = []
		method_ops = self.xman.read_all_ops(self.curr_note.get())
		#print(list(method_ops))
		if len(method_ops):#Si existe contenido por sumar
			for i in method_ops:
				method_data.append([i[0], float(i[1]), float(i[2]), float(i[3]), float(i[4]), int(i[5])])
			self.cold_data = osc.operation(method_data, self.curr_sim, self.scale)
			if (len(self.temp_data) and self.imp_temp.get()):
				self.cold_data = osc.op_arrays(self.cold_data, self.temp_data, False)
			self.f_cold_data = self.fou.get_FFT(self.cold_data, self.spectre)
			self.f_cold_data = self.f_cold_data / max(self.f_cold_data)
	def update_temp_data(self):
		self.adsr.set_parameters(self.tv[0],self.tv[1],self.tv[2],self.tv[3],self.tv[4],self.tv[5],self.tv[6])
		self.temp_data = self.adsr.get_template(self.rate_sim, self.curr_sim)
	def update_audio_data(self):
		b,self.curr_sim = ut.round_i(self.begin_audio * self.rate_sim), ut.round_i(self.time_audio * self.rate_sim)
		self.fou.set_sample_area_at_sample(begin = b, lenght_sam = self.curr_sim, spectre=self.spectre)
		self.audio_data = self.fou.get_amp_sample()
		self.audio_data = self.audio_data / max(self.audio_data)
		self.f_audio_data = self.fou.get_fft_sample()
		self.f_audio_data = self.f_audio_data / max(self.f_audio_data)

	def refresh_screen(self,*args):
		a_samp = np.arange(0,len(self.audio_data)) / self.rate_sim
		x_samp = np.arange(0,self.curr_sim) / self.rate_sim
		f_samp = np.arange(0,self.spectre)

		self.ax.cla()
		if(self.show_four.get()):
			if(self.show_audio.get() and len(self.f_audio_data)):
				self.ax.plot(f_samp, self.f_audio_data, color = 'purple')
			if(self.show_cold.get() and len(self.f_cold_data)):
				self.ax.plot(f_samp, self.f_cold_data, color = 'orange')
			if(self.show_hot.get() and len(self.f_hot_data)):
				self.ax.plot(f_samp, self.f_hot_data, color = 'blue')
		else:
			if(self.show_audio.get() and len(self.audio_data)):
				self.ax.plot(a_samp, self.audio_data, color = 'green')
			if(self.show_cold.get() and len(self.cold_data)):
				self.ax.plot(x_samp, self.cold_data, color = 'cyan')
			if(self.show_temp.get() and len(self.temp_data)):
				self.ax.plot(x_samp, self.temp_data, color = 'gold')
			if(self.show_hot.get() and len(self.hot_data) and (self.curr_expr.get() != 'Operacion')):
				self.ax.plot(x_samp, self.hot_data, color = 'red')
		self.cv_main.draw()
	#Controladores
	def val_meth_list(self, *args):
		if self.act_meth_val:
			self.iv = [1.0, 1.0, 0.0, 0.0, 20]
			self.begin_audio = 0.0
			self.time_audio = 1.0
			try:
				self.iv[0] = float(self.value_a.get())
			except ValueError:
				pass
			try:
				self.iv[1] = float(self.value_b.get())
			except ValueError:
				pass
			try:
				self.iv[2] = float(self.value_c.get())
			except ValueError:
				pass
			try:
				self.iv[3] = float(self.value_d.get())
			except ValueError:
				pass
			try:
				self.iv[4] = int(self.value_e.get())
			except ValueError:
				pass
			try:
				self.begin_audio = float(self.value_ab.get())
			except ValueError:
				pass
			try:
				self.time_audio = float(self.value_at.get())
			except ValueError:
				pass

			if len(self.audio_data):
				self.update_audio_data()
			if (self.curr_expr.get() != 'Operacion') :
				self.update_hot_data()
			self.update_temp_data()
			self.update_cold_data()
			self.refresh_screen()
	def val_temp_list(self, *args):
		if self.act_temp_val:
			self.tv = [0.2, 0.2, 0.4, 0.2, 1.0, 0.6, 0.4]
			try:
				if float(self.value_att.get()) > 0.0:
					self.tv[0] = float(self.value_att.get())
			except ValueError:
				pass
			try:
				if float(self.value_dec.get()) > 0.0:
					self.tv[1] = float(self.value_dec.get())
			except ValueError:
				pass
			try:
				if float(self.value_sus.get()) > 0.0:
					self.tv[2] = float(self.value_sus.get())
			except ValueError:
				pass
			try:
				if float(self.value_rel.get()) > 0.0:
					self.tv[3] = float(self.value_rel.get())
			except ValueError:
				pass
			try:
				if float(self.value_pic.get()) > 0.0:
					self.tv[4] = float(self.value_pic.get())
			except ValueError:
				pass
			try:
				if float(self.value_stb.get()) > 0.0:
					self.tv[5] = float(self.value_stb.get())
			except ValueError:
				pass
			try:
				if float(self.value_res.get()) > 0.0:
					self.tv[6] = float(self.value_res.get())
			except ValueError:
				pass

			self.update_temp_data()
			self.update_cold_data()

	def expr_listener(self,*args):
		self.act_meth_val = False
		self.iv = [1.0, 1.0, 0.0, 0.0, 0]
		self.txb_e.config(state=tk.DISABLED)
		self.value_a.set(value='1.0')
		self.value_b.set(value='1.0')
		self.value_c.set(value='0.0')
		self.value_d.set(value='0.0')
		self.value_e.set(value='0')

		self.tag_a.set(value='Amplitud')
		self.tag_b.set(value='Frecuencia')
		self.tag_c.set(value='Fase')
		self.tag_d.set(value='Origen')
		self.tag_e.set(value='')

		if(self.curr_expr.get() == "SierraAna"):
			self.txb_e.config(state=tk.NORMAL)
			self.iv = [1.0, 1.0, 0.0, 0.0, 20]
			self.tag_e.set(value='Precision')
			self.value_e.set(value='20')
		if(self.curr_expr.get() == "SierraOpt"):
			self.value_b.set(value='0.16')
			self.iv = [1.0, 0.16, 0.0, 0.0, 0]
		if(self.curr_expr.get() == "Ruido"):
			self.tag_a.set(value='Minimo')
			self.tag_b.set(value='Maximo')
			self.tag_c.set(value='Origen')
			self.tag_d.set(value='Semilla')
			self.value_a.set(value='-1.0')
			self.value_b.set(value='1.0')
			self.iv = [-1.0, 1.0, 0.0, 0.0, 0]
		self.act_meth_val = True

		self.update_hot_data()
		self.refresh_screen()
	def load_temp(self, data):
		self.act_temp_val = False
		self.curr_temp.set(value='ASDR')
		self.value_att.set(value=data[0])
		self.value_dec.set(value=data[1])
		self.value_sus.set(value=data[2])
		self.value_rel.set(value=data[3])
		self.value_pic.set(value=data[4])
		self.value_stb.set(value=data[5])
		self.act_temp_val = True
		self.value_res.set(value=data[6])

	def set_method(self):
		if (self.curr_expr.get() == 'Operacion'):
			return 0
		self.lb_sum.insert(self.lb_sum.size(),'{:.2f}{}({:.2f}w + {:.2f}) +{:.2f}'.format(
		float(self.value_a.get()), self.curr_expr.get(), float(self.value_b.get()), float(self.value_c.get()), float(self.value_d.get()) ))
		self.xman.add_ops(self.curr_note.get(), self.get_co(self.curr_expr.get()), self.iv)
		
		self.update_cold_data()
		self.refresh_screen()
	def del_method(self):
		i = 0
		try:
			i = self.lb_sum.curselection()[0]
		except IndexError:
			if self.xman.get_size_ops(self.curr_note.get()) == 0:	
				return 0
		self.xman.del_ops(self.curr_note.get(), i)
		self.lb_sum.delete(i)
		self.update_cold_data()
		self.refresh_screen()	
	def mod_method(self):
		i = 0
		try:
			i = self.lb_sum.curselection()[0]
		except IndexError:
			if self.xman.get_size_ops(self.curr_note.get()) == 0:	
				return 0
		xml_data = self.xman.read_ops(self.curr_note.get(), i)
		self.curr_expr.set(self.get_co(xml_data[0]))
		self.value_a.set(xml_data[1])
		self.value_b.set(xml_data[2])
		self.value_c.set(xml_data[3])
		self.value_d.set(xml_data[4])
		self.value_e.set(xml_data[5])
		self.lb_sum.delete(i)
		self.update_cold_data()
		self.refresh_screen()

	def save_xml_file(self):
		if self.imp_temp.get():
			self.xman.set_template(self.curr_note.get(),self.tv)
		file_inst = filedialog.asksaveasfilename(initialdir='./export', filetypes=[("XML data", ".xml")])
		if file_inst:
			if file_inst.find('.xml') == -1:
				file_inst += '.xml'
			self.xman.set_path(file_inst)
			self.xman.bake()
	def load_xml_file(self):
		file_inst = filedialog.askopenfilename(initialdir='./export', filetypes=[("XML data", ".xml")])
		if file_inst:
			self.xman.load_archive(file_inst)
			self.lb_sum.delete(0, tk.END)
			data = self.xman.read_all_ops(self.curr_note.get())
			for x in data:
				self.lb_sum.insert(self.lb_sum.size(),
				'{:.2f}{}({:.2f}w + {:.2f}) +{:.2f}'.format(
				float(x[1]), self.get_co(x[0]), float(x[2]), float(x[3]), float(x[4])))
			self.load_temp(self.xman.read_temp(self.curr_note.get()))
			if(self.show_cold.get()):
				self.update_cold_data()
			self.refresh_screen()

	def load_wav_file(self):
		self.btn_a_graf.config(state=tk.DISABLED)
		self.txb_ab.config(state=tk.DISABLED)
		self.txb_at.config(state=tk.DISABLED)
		path = filedialog.askopenfile(initialdir='./samples', filetypes=[("WAV audio", ".wav")])
		if path.name:
			self.fou.load_archive(path.name)
			self.rate_sim = self.fou.get_rate()
			self.scale = (2 * np.pi) / self.rate_sim
			self.act_meth_val = False
			self.value_ab.set('0.0')
			self.value_at.set('{:.2f}'.format(self.fou.get_duration_time()))
			self.time_audio = self.fou.get_duration_time()
			self.act_meth_val = True
			self.update_audio_data()
			self.update_hot_data()
			self.update_temp_data()
			self.update_cold_data()
			self.refresh_screen()
			self.btn_a_graf.config(state=tk.NORMAL)
			self.btn_armonic.config(state=tk.NORMAL)
			self.txb_ab.config(state=tk.NORMAL)
			self.txb_at.config(state=tk.NORMAL)

	def graphic_audio(self):
		self.fou.graphic_amp_plot()
	def graphic_armonics(self):
		self.fou.graphic_fft_plot()

	#Layout de la App
	def create_vars(self): 
		# Declaración de variables de control  
		self.curr_expr = tk.StringVar(value='Operacion')
		self.value_a = tk.StringVar(value='1.0')
		self.value_b = tk.StringVar(value='1.0')
		self.value_c = tk.StringVar(value='0.0')
		self.value_d = tk.StringVar(value='0.0')
		self.value_e = tk.StringVar(value='20')
		self.value_z = tk.StringVar(value='1.0')
		self.value_ab = tk.StringVar(value='0.0')
		self.value_at = tk.StringVar(value='1.0')
		self.tag_a = tk.StringVar(value='Amplitud')
		self.tag_b = tk.StringVar(value='Frecuencia')
		self.tag_c = tk.StringVar(value='Fase')
		self.tag_d = tk.StringVar(value='Origen')
		self.tag_e = tk.StringVar(value='Precision')

		self.curr_temp = tk.StringVar(value='None')
		self.value_att = tk.StringVar(value='0.2')
		self.value_dec = tk.StringVar(value='0.2')
		self.value_sus = tk.StringVar(value='0.4')
		self.value_rel = tk.StringVar(value='0.2')
		self.value_pic = tk.StringVar(value='1.0')
		self.value_stb = tk.StringVar(value='0.6')
		self.value_res = tk.StringVar(value='0.4')

		self.curr_note = tk.StringVar(value='la')

		self.show_audio = tk.BooleanVar(value=True)
		self.show_hot = tk.BooleanVar(value=True)
		self.show_cold = tk.BooleanVar(value=False)
		self.show_temp = tk.BooleanVar(value=False)
		self.show_four = tk.BooleanVar(value=False)
		self.imp_temp = tk.BooleanVar(value=False)

		self.curr_expr.trace('w',self.expr_listener)

		self.value_a.trace('w',self.val_meth_list)
		self.value_b.trace('w',self.val_meth_list)
		self.value_c.trace('w',self.val_meth_list)
		self.value_d.trace('w',self.val_meth_list)
		self.value_e.trace('w',self.val_meth_list)
		self.value_z.trace('w',self.val_meth_list)
		self.value_ab.trace('w',self.val_meth_list)
		self.value_at.trace('w',self.val_meth_list)

		self.curr_temp.trace('w',self.val_temp_list)
		self.imp_temp.trace('w',self.val_temp_list)
		self.value_att.trace('w',self.val_temp_list)
		self.value_dec.trace('w',self.val_temp_list)
		self.value_sus.trace('w',self.val_temp_list)
		self.value_rel.trace('w',self.val_temp_list)
		self.value_pic.trace('w',self.val_temp_list)
		self.value_stb.trace('w',self.val_temp_list)
		self.value_res.trace('w',self.val_temp_list)

		self.show_hot.trace('w',self.refresh_screen)
		self.show_temp.trace('w',self.refresh_screen)
		self.show_cold.trace('w',self.refresh_screen)
		self.show_audio.trace('w',self.refresh_screen)
		self.show_four.trace('w',self.refresh_screen)
	def create_widgets(self):
		#Frames
		self.nt_expression = ttk.Notebook(self.master)

		self.fr_method = tk.Frame(self.nt_expression)
		self.fr_method.place(height=230, width=850)
		self.fr_template = tk.Frame(self.nt_expression)
		self.fr_template.place(height=230, width=850)
		self.fr_control = tk.Frame(self.master)
		self.fr_control.place(x=865,y=805,height=230, width=660)
		
		self.nt_expression.add(self.fr_template, text = "Plantilla")
		self.nt_expression.add(self.fr_method, text = "Método")
		self.nt_expression.place(x=10,y=805, height=230, width=850)

		#Canvas
		self.cv_main.get_tk_widget().place(x=10, height=800, width=1900)
		
		#Guardado de Archivo XML
		self.btn_safe = tk.Button(self.fr_control, text="Guardar", command=self.save_xml_file)
		self.btn_safe.place(y=25,width=110)
		self.btn_load = tk.Button(self.fr_control, text="Cargar", command=self.load_xml_file)
		self.btn_load.place(y=50,width=110)
		#Opciones del Lienzo
		self.chb_hot = tk.Checkbutton(self.fr_control, text="Actual", fg='red', variable=self.show_hot, onvalue=True, offvalue=False)
		self.chb_hot.place(x=115,y=10)
		self.chb_cold = tk.Checkbutton(self.fr_control, text="Modelo", fg='cyan', variable=self.show_cold, onvalue=True, offvalue=False)
		self.chb_cold.place(x=115,y=30)
		self.chb_cold = tk.Checkbutton(self.fr_control, text="Plantilla", fg='gold', variable=self.show_temp, onvalue=True, offvalue=False)
		self.chb_cold.place(x=115,y=50)
		
		#Controlador de audio
		self.chb_audio = tk.Checkbutton(self.fr_control, text="Audio", fg='green', variable=self.show_audio, onvalue=True, offvalue=False)
		self.chb_audio.place(x=115,y=70)
		self.chb_four = tk.Checkbutton(self.fr_control, text="Fourier", fg='purple', variable=self.show_four, onvalue=True, offvalue=False)
		self.chb_four.place(x=115,y=90)
		self.lbl_audio = tk.Label(self.fr_control, text='Control de Audio')
		self.lbl_audio.place(x=230,y=10)
		self.lbl_ab = tk.Label(self.fr_control, text='Inicio')
		self.lbl_ab.place(x=220,y=25)
		self.txb_ab = tk.Entry(self.fr_control, bd=4, textvariable=self.value_ab, state=tk.DISABLED)
		self.txb_ab.place(x=220,y=42, width=55)
		self.lbl_at = tk.Label(self.fr_control, text='Tiempo')
		self.lbl_at.place(x=280,y=25)
		self.txb_at = tk.Entry(self.fr_control, bd=4, textvariable=self.value_at, state=tk.DISABLED)
		self.txb_at.place(x=280,y=42, width=55)
		self.btn_a_graf = tk.Button(self.fr_control, text="Graficar Audio", state=tk.DISABLED, command=self.graphic_audio)
		self.btn_a_graf.place(x=220,y=95, width=115)
		self.btn_armonic = tk.Button(self.fr_control, text="Graficar Armonicos", state=tk.DISABLED, command=self.graphic_armonics)
		self.btn_armonic.place(x=220,y=120, width=115)
		self.btn_audio = tk.Button(self.fr_control, text="Cargar", command=self.load_wav_file)
		self.btn_audio.place(x=220,y=70,width=115)

		self.cbx_note = ttk.Combobox(self.fr_control,values=['la','la#','si','do','do#','re','re#','mi','fa','fa#','sol','sol#'],textvariable=self.curr_note)
		self.cbx_note.place(x=340, y=42, width=40)

		#Panel de metodos
		self.cbx_ops = ttk.Combobox(self.fr_method,values=["Seno","Cuadratica","Triangular","SierraAna","SierraOpt","Ruido"],textvariable=self.curr_expr)
		self.cbx_ops.set("Operacion")
		self.cbx_ops.place(x=270, y=10, width=85)

		self.lbl_a = tk.Label(self.fr_method, textvariable=self.tag_a)
		self.lbl_a.place(x=350,y=40, width=85)
		self.txb_a = tk.Entry(self.fr_method, bd=4, textvariable=self.value_a)
		self.txb_a.place(x=270,y=40, width=85)

		self.lbl_b = tk.Label(self.fr_method, textvariable=self.tag_b)
		self.lbl_b.place(x=350,y=65, width=85)
		self.txb_b = tk.Entry(self.fr_method, bd=4, textvariable=self.value_b)
		self.txb_b.place(x=270,y=65, width=85)

		self.lbl_c = tk.Label(self.fr_method, textvariable=self.tag_c)
		self.lbl_c.place(x=350,y=90, width=85)
		self.txb_c = tk.Entry(self.fr_method, bd=4, textvariable=self.value_c)
		self.txb_c.place(x=270,y=90, width=85)

		self.lbl_d = tk.Label(self.fr_method, textvariable=self.tag_d)
		self.lbl_d.place(x=350,y=115, width=85)
		self.txb_d = tk.Entry(self.fr_method, bd=4, textvariable=self.value_d)
		self.txb_d.place(x=270,y=115, width=85)

		self.lbl_e = tk.Label(self.fr_method, textvariable=self.tag_e)
		self.lbl_e.place(x=350,y=140, width=85)
		self.txb_e = tk.Entry(self.fr_method, bd=4, textvariable=self.value_e, state=tk.DISABLED)
		self.txb_e.place(x=270,y=140, width=85)

		self.btn_add_sum = tk.Button(self.fr_method, text="Añadir", command=self.set_method)
		self.btn_add_sum.place(x=10,width=85)
		self.btn_mod_sum = tk.Button(self.fr_method, text="Editar", command=self.mod_method)
		self.btn_mod_sum.place(x=95,width=85)
		self.btn_rmv_sum = tk.Button(self.fr_method, text="Quitar", command=self.del_method)
		self.btn_rmv_sum.place(x=180,width=85)

		self.sb_sum = tk.Scrollbar(self.fr_method)
		self.sb_sum.place(x=10, y=30, height=175)
		self.lb_sum = tk.Listbox(self.fr_method, yscrollcommand = self.sb_sum.set)
		self.lb_sum.place(x=25, y=30, height=175, width=240)
		self.sb_sum.config(command=self.lb_sum.yview)

		#Panel de plantilla
		self.cbx_type_temp = ttk.Combobox(self.fr_template,values=["None","ADSR"], textvariable = self.curr_temp)
		self.cbx_type_temp.set("None")
		self.cbx_type_temp.place(x=10,y=10,width=75)

		self.lbl_attack = tk.Label(self.fr_template, text = 'Ascenso')
		self.lbl_attack.place(x=10,y=35, width=50)
		self.txb_attack = tk.Entry(self.fr_template, bd=4, textvariable = self.value_att)
		self.txb_attack.place(x=10,y=55, width=50)

		self.lbl_decay = tk.Label(self.fr_template, text = 'Caída')
		self.lbl_decay.place(x=70,y=35, width=50)
		self.txb_decay = tk.Entry(self.fr_template, bd=4, textvariable = self.value_dec)
		self.txb_decay.place(x=70,y=55, width=50)

		self.lbl_sustain = tk.Label(self.fr_template, text = 'Sostén')
		self.lbl_sustain.place(x=130,y=35, width=50)
		self.txb_sustain = tk.Entry(self.fr_template, bd=4, textvariable = self.value_sus)
		self.txb_sustain.place(x=130,y=55, width=50)

		self.lbl_release = tk.Label(self.fr_template, text = 'Liberar')
		self.lbl_release.place(x=190,y=35, width=50)
		self.txb_release = tk.Entry(self.fr_template, bd=4, textvariable = self.value_rel)
		self.txb_release.place(x=190,y=55, width=50)

		self.lbl_pitch = tk.Label(self.fr_template, text = 'Pico')
		self.lbl_pitch.place(x=10,y=80, width=50)
		self.txb_pitch = tk.Entry(self.fr_template, bd=4, textvariable = self.value_pic)
		self.txb_pitch.place(x=10,y=100, width=50)

		self.lbl_stable = tk.Label(self.fr_template, text = 'Estable')
		self.lbl_stable.place(x=70,y=80, width=50)
		self.txb_stable = tk.Entry(self.fr_template, bd=4, textvariable = self.value_stb)
		self.txb_stable.place(x=70,y=100, width=50)

		self.lbl_rest = tk.Label(self.fr_template, text = 'Descenso')
		self.lbl_rest.place(x=130,y=80, width=50)
		self.txb_rest = tk.Entry(self.fr_template, bd=4, textvariable = self.value_res)
		self.txb_rest.place(x=130,y=100, width=50)

		self.btn_applay = tk.Checkbutton(self.fr_template, text='Aplicar', variable=self.imp_temp, onvalue=True, offvalue=False)
		self.btn_applay.place(x=250,y=55, width=50)

root = tk.Tk()

def close():
	plt.close('all')
	root.destroy()

def main():
	root.wm_title("Tuner by JoGEHrt V_0.6.9")
	root.wm_protocol('WM_DELETE_WINDOW',close)
	app = Application(root)
	app.mainloop()

if __name__ == "__main__":
	main()