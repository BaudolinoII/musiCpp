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
		self.scroll_x = tk.Scrollbar(self.master)
		self.scroll_y = tk.Scrollbar(self.master)
		self.txb_e = tk.Entry(self.master)
		self.txb_ab = tk.Entry(self.master)
		self.txb_at = tk.Entry(self.master)
		self.txb_sb = tk.Entry(self.master)
		self.txb_st = tk.Entry(self.master)
		self.btn_audio = tk.Button(self.master)
		self.btn_armonic = tk.Button(self.master)
		self.btn_gnr_arm = tk.Button(self.master)
		self.cbx_note = ttk.Combobox(self.master)
		self.sec_win = tk.Toplevel()
		self.sec_win.destroy()

		self.file_inst = '../instrumento.xml'
		self.file_audio = '../sonido.wav'
		self.xman = XML_Manager(self.file_inst)
		self.fou = Fourier()
		self.adsr = tp.ADSR()

		#Valores de simulacion
		self.begin_audio = 0.0
		self.time_audio = 1.0
		self.time_sim = 1.0
		self.curr_sim = 44100
		self.begin_spectre = 0
		self.lenght_spectre = 1000

		self.iv = [1.0, 1.0, 0.0, 0.0, 20]
		self.tv = [0.2, 0.2, 0.4, 0.2, 1.0, 0.6, 0.4]
		self.act_meth_val = True
		self.act_temp_val = True

		self.rate_sample = 44100
		#Buffers		
		self.hot_data = []
		self.cold_data = []
		self.temp_data = []
		self.audio_data = []
		self.f_audio_data = []
		self.f_cold_data = []
		self.f_hot_data = []

		self.occ_scale = ['la','la#','si','do','do#','re','re#','mi','fa','fa#','sol','sol#']

		self.create_vars()
		self.create_widgets()
		self.master.geometry("900x860")
		self.master.state('zoomed') # "1920x980"
	#Métodos Complementarios
	def push_graphic(self, data, scalar = 1.0, color_grap='red'):
		self.ax.plot(np.arange(0.0, float(len(data)), 1.0) / scalar, data, color=color_grap)
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
	#Actualizacion de valores en simulacion
	def update_hot_data(self):
		input_data = [[self.get_co(self.curr_expr.get()), self.iv[0], self.iv[1], self.iv[2], self.iv[3], self.iv[4]]]
		self.hot_data = osc.operation(input_data, self.curr_sim, self.rate_sample)
		self.f_hot_data = self.fou.get_FFT(self.hot_data, self.begin_spectre ,self.lenght_spectre)
		if max(self.f_hot_data):
			self.f_hot_data = self.f_hot_data * ( self.iv[0] / max(self.f_hot_data))
	def update_cold_data(self):
		method_data = []
		method_ops = self.xman.read_all_ops(self.curr_note.get())
		if len(method_ops):#Si existe contenido por sumar
			for i in method_ops:
				method_data.append([i[0], float(i[1]), float(i[2]), float(i[3]), float(i[4]), int(i[5])])
			self.cold_data = osc.operation(method_data, self.curr_sim, self.rate_sample)
			if (len(self.temp_data) and (self.curr_temp.get() != 'None')):
				self.cold_data = osc.op_arrays(self.cold_data, self.temp_data, False)
			self.f_cold_data = self.fou.get_FFT(self.cold_data, self.begin_spectre, self.lenght_spectre)
			self.f_cold_data = self.f_cold_data / max(self.f_cold_data)
	def update_temp_data(self):
		if(self.curr_temp.get() == 'ADSR'):
			self.adsr.set_parameters(self.tv[0],self.tv[1],self.tv[2],self.tv[3],self.tv[4],self.tv[5],self.tv[6])
			self.temp_data = self.adsr.get_template(self.curr_sim)
		self.xman.set_template(self.curr_note.get(),self.curr_temp.get(), self.tv)
	def update_audio_data(self):
		b, a_sim = ut.round_i(self.begin_audio * self.rate_sample), ut.round_i(self.time_audio * self.rate_sample)
		if(self.time_audio > self.fou.get_duration_time()):
			a_sim = ut.round_i(self.fou.get_duration_time() * self.rate_sample)
		self.fou.set_sample_area_at_sample(begin = b, lenght_sam = a_sim, bs=self.begin_spectre, spectre=self.lenght_spectre)
		self.audio_data = self.fou.get_amp_sample()
		self.f_audio_data = self.fou.get_fft_sample()
		self.f_audio_data = self.f_audio_data / max(self.f_audio_data)
	#Escuchas de Variables
	def refresh_screen(self,*args):
		self.ax.cla()
		if(self.show_four.get()):
			if(self.show_audio.get() and len(self.f_audio_data)):
				self.push_graphic(self.f_audio_data, 1.0, 'purple')
			if(self.show_cold.get() and len(self.f_cold_data)):
				self.push_graphic(self.f_cold_data, 1.0, 'orange')
			if(self.show_hot.get() and len(self.f_hot_data) and (self.curr_expr.get() != 'Operacion')):
				self.push_graphic(self.f_hot_data, 1.0, 'blue')
		else:
			if(self.show_audio.get() and len(self.audio_data)):
				self.push_graphic(self.audio_data, self.rate_sample, 'green')
			if(self.show_cold.get() and len(self.cold_data)):
				self.push_graphic(self.cold_data, self.rate_sample, 'cyan')
			if(self.show_temp.get() and len(self.temp_data)):
				self.push_graphic(self.temp_data, self.rate_sample, 'gold')
			if(self.show_hot.get() and len(self.hot_data) and (self.curr_expr.get() != 'Operacion')):
				self.push_graphic(self.hot_data, self.rate_sample,'red')
		self.cv_main.draw()
	
	def val_meth_list(self, *args):
		if self.act_meth_val:
			self.iv = [1.0, 1.0, 0.0, 0.0, 20]
			self.begin_audio = 0.0
			self.time_audio = 1.0
			self.time_sim = 1.0
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
			try:
				self.time_sim = float(self.value_sd.get())
			except ValueError:
				pass
			try:
				self.begin_spectre = int(self.value_sb.get())
			except ValueError:
				pass
			try:
				self.lenght_spectre = int(self.value_st.get())
			except ValueError:
				pass

			self.curr_sim = ut.round_i(self.time_sim * self.rate_sample)

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
				if float(self.value_att.get()) >= 0.0:
					self.tv[0] = float(self.value_att.get())
			except ValueError:
				pass
			try:
				if float(self.value_dec.get()) >= 0.0:
					self.tv[1] = float(self.value_dec.get())
			except ValueError:
				pass
			try:
				if float(self.value_sus.get()) >= 0.0:
					self.tv[2] = float(self.value_sus.get())
			except ValueError:
				pass
			try:
				if float(self.value_rel.get()) >= 0.0:
					self.tv[3] = float(self.value_rel.get())
			except ValueError:
				pass
			try:
				if float(self.value_pic.get()) >= 0.0:
					self.tv[4] = float(self.value_pic.get())
			except ValueError:
				pass
			try:
				if float(self.value_stb.get()) >= 0.0:
					self.tv[5] = float(self.value_stb.get())
			except ValueError:
				pass
			try:
				if float(self.value_res.get()) >= 0.0:
					self.tv[6] = float(self.value_res.get())
			except ValueError:
				pass

			self.update_temp_data()
			self.update_cold_data()
			self.refresh_screen()
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
		self.tag_c.set(value='Amp_Vibrato')
		self.tag_d.set(value='Freq_Vibrato')
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

	def update_lb_meth(self, *args):
		self.lb_sum.delete(0, tk.END)
		for x in self.xman.read_all_ops(self.curr_note.get()):
			self.lb_sum.insert(self.lb_sum.size(),
			'{:.2f}{}({:.2f}wt + args); args = {:.2f} * {:.2f} * Seno({:.2f}wt)'.format(
			float(x[1]), self.get_co(x[0]), float(x[2]), float(x[1]), float(x[3]), float(x[4])))
		self.load_temp(self.xman.read_temp(self.curr_note.get()))
	def update_xml_format(self, *args):
		if self.mode_note.get():
			self.tag_bmn.set('Individual')
			self.cbx_note.config(state=tk.DISABLED)
			self.curr_note.set('all')
		else:
			self.tag_bmn.set('Universal')
			self.cbx_note.config(state=tk.NORMAL)
			self.curr_note.set('la')
	def get_armonic_freq(self, *args):
		self.value_b.set('{:.2f}'.format(ut.armonic_freq(int(self.value_id.get()))))
	def get_axis_id_freq(self, *args):
		try:
			self.tag_axis_freq.set('Frecuencia {:.2f} Hz'.format(ut.armonic_freq(int(self.axis_id_note.get()))))
		except ValueError:
			pass
	#Controladores
	def set_method(self):
		if (self.curr_expr.get() == 'Operacion'):
			return 0
		self.xman.add_ops(self.curr_note.get(), self.get_co(self.curr_expr.get()), self.iv)
		self.update_lb_meth()
		self.update_cold_data()
		self.refresh_screen()
	def del_method(self):
		for item in self.lb_sum.curselection():
			self.xman.del_ops(self.curr_note.get(), item)
			print(item)
		self.update_lb_meth()
		self.update_cold_data()
		self.refresh_screen()	
	def mod_method(self):
		for item in self.lb_sum.curselection():
			xml_data = self.xman.read_ops(self.curr_note.get(), item)
			self.xman.del_ops(self.curr_note.get(), item)
			self.curr_expr.set(self.get_co(xml_data[0]))
			self.value_a.set(xml_data[1])
			self.value_b.set(xml_data[2])
			self.value_c.set(xml_data[3])
			self.value_d.set(xml_data[4])
			self.value_e.set(xml_data[5])
		self.update_lb_meth()
		self.update_cold_data()
		self.refresh_screen()

	def save_xml_file(self):
		file_inst = filedialog.asksaveasfilename(initialdir='./export', filetypes=[("XML data", ".xml")])
		if file_inst:
			if file_inst.find('.xml') == -1:
				file_inst += '.xml'
			self.xman.set_metadata(self.value_name.get(), float(self.value_vol.get()), float(self.value_durM.get()))
			self.xman.set_path(file_inst)
			self.xman.bake()
	def load_xml_file(self):
		file_inst = filedialog.askopenfilename(initialdir='./export', filetypes=[("XML data", ".xml")])
		if file_inst:
			self.xman.load_archive(file_inst)
			self.mode_note.set(self.xman.get_mode())
			mtdt = self.xman.get_metadata()
			self.value_name.set(mtdt[0])
			self.value_vol.set(mtdt[1])
			self.value_durM.set(mtdt[2])
			self.update_cold_data()
			self.refresh_screen()

	def load_temp(self, data):
		self.act_temp_val = False
		self.curr_temp.set(value=data[0])
		self.value_att.set(value=data[1])
		self.value_dec.set(value=data[2])
		self.value_sus.set(value=data[3])
		self.value_rel.set(value=data[4])
		self.value_pic.set(value=data[5])
		self.value_stb.set(value=data[6])
		self.act_temp_val = True
		self.value_res.set(value=data[7])
	def load_wav_file(self):
		self.txb_ab.config(state=tk.DISABLED)
		self.txb_at.config(state=tk.DISABLED)
		self.txb_sb.config(state=tk.DISABLED)
		self.txb_st.config(state=tk.DISABLED)
		self.btn_audio.config(state=tk.DISABLED)
		self.btn_gnr_arm.config(state=tk.DISABLED)
		path = filedialog.askopenfile(initialdir='./samples', filetypes=[("WAV audio", ".wav")])
		if path.name:
			self.fou.load_archive(path.name)
			self.rate_sample = self.fou.get_rate()
			self.act_meth_val = False
			self.value_ab.set('0.0')
			self.value_at.set('{:.2f}'.format(self.fou.get_duration_time()))
			self.value_sd.set('{:.2f}'.format(self.fou.get_duration_time()))
			self.value_sb.set('0')
			self.value_st.set('{}'.format(self.fou.get_lenght_spectre()))
			self.act_meth_val = True
			self.time_audio = self.fou.get_duration_time()
			self.time_sim = self.time_audio
			self.curr_sim = ut.round_i(self.time_sim * self.rate_sample)
			self.begin_spectre = 0
			self.lenght_spectre = self.fou.get_lenght_spectre()
			self.update_audio_data()
			self.update_hot_data()
			self.update_temp_data()
			self.update_cold_data()
			self.refresh_screen()
			self.txb_ab.config(state=tk.NORMAL)
			self.txb_at.config(state=tk.NORMAL)
			self.txb_sb.config(state=tk.NORMAL)
			self.txb_st.config(state=tk.NORMAL)
			self.btn_audio.config(state=tk.NORMAL)
			self.btn_gnr_arm.config(state=tk.NORMAL)

	def ask_for_ga_params(self):
		self.sec_win = tk.Toplevel()
		self.sec_win.title('Generar Armónicos')
		self.sec_win.geometry('300x240')

		lbl_aga_op = tk.Label(self.sec_win, text='Tipo de Operacion')
		lbl_aga_op.grid(column=0,row=0)
		cbx_aga_ops = ttk.Combobox(self.sec_win, values=["Seno","Cuadratica","Triangular","SierraAna","SierraOpt"], textvariable= self.op_ga)
		cbx_aga_ops.set("Seno")
		cbx_aga_ops.grid(column=0,row=1)

		lbl_aga_fl = tk.Label(self.sec_win, text='Flud')
		lbl_aga_fl.grid(column=1,row=0)
		tbx_aga_fl = tk.Entry(self.sec_win, textvariable = self.value_flud)
		tbx_aga_fl.grid(column=1,row=1)

		btn_aga_ga = tk.Button(self.sec_win, text='Generar', command=self.generate_armonics)
		btn_aga_ga.grid(column=0,row=2)
		btn_aga_ex = tk.Button(self.sec_win, text='Cancelar', command=self.sec_win.destroy)
		btn_aga_ex.grid(column=1,row=2)
	def ask_for_cf_params(self):
		self.sec_win = tk.Toplevel()
		self.sec_win.title('Generar Notas')
		self.sec_win.geometry('300x240')

		lbl_notes = tk.Label(self.sec_win,text='Nota Base')
		lbl_notes.grid(column=0, row=0, sticky='N')
		ax_notes = tk.Entry(self.sec_win, textvariable=self.axis_id_note)
		ax_notes.grid(column=0, row=1, sticky='N')
		lbl_freq = tk.Label(self.sec_win,textvariable=self.tag_axis_freq)
		lbl_freq.grid(column=0, row=2, sticky='N')

		if not self.mode_note.get():
			lbl_tag = tk.Label(self.sec_win,text='Etiqueta')
			lbl_tag.grid(column=1, row=0, sticky='N')
			ax_notes = ttk.Combobox(self.sec_win,
			values=self.occ_scale, textvariable=self.axis_tag_note)
			ax_notes.grid(column=1, row=1, sticky='N')

		lbl_notes = tk.Label(self.sec_win,text='Precision')
		lbl_notes.grid(column=0, row=3, sticky='N')
		ax_notes = tk.Entry(self.sec_win, textvariable=self.pres_gn)
		ax_notes.grid(column=0, row=4 ,sticky='N')

		self.chb_hot = tk.Checkbutton(self.sec_win, text="Plantilla Total", variable=self.with_temp, onvalue=True, offvalue=False)
		self.chb_hot.grid(column = 1, row = 2, sticky='N')

		lbl_aga_op = tk.Label(self.sec_win, text='Tipo de Operacion')
		lbl_aga_op.grid(column=1,row=3)
		cbx_aga_ops = ttk.Combobox(self.sec_win, values=["Seno","Cuadratica","Triangular","SierraAna","SierraOpt"], textvariable= self.op_ga)
		cbx_aga_ops.set("Seno")
		cbx_aga_ops.grid(column=1,row=4)

		btn_gnr_nts = tk.Button(self.sec_win,text='Generar',command=self.generate_notes)
		btn_gnr_nts.grid(column=0, row=5, sticky='N')
		btn_gnr_nts = tk.Button(self.sec_win,text='Cancelar',command=self.sec_win.destroy)
		btn_gnr_nts.grid(column=1, row=5, sticky='N')

	def generate_notes(self):
		if self.mode_note.get():
			self.xman.del_all_ops('all')
			if self.with_temp.get():
					self.xman.set_template('all','ADSR',[0.2, 0.2, 0.4, 0.2, 1.0, 0.6, 0.4])
			for i in range(0, int(self.pres_gn.get())):
				self.xman.add_ops('all', self.get_co(self.op_ga.get()), 
				[float(1.0 / (2.0 ** i)), ut.armonic_freq(int(self.axis_id_note.get()) + (12 * i)), 0.0, 0.0, 10])
		else:
			begin = self.occ_scale.index(self.axis_tag_note.get())
			for note in self.occ_scale:
				self.xman.del_all_ops(note)
				if self.with_temp.get():
						self.xman.set_template(note,'ADSR',[0.2, 0.2, 0.4, 0.2, 1.0, 0.6, 0.4])
				for i in range(0, int(self.pres_gn.get())):
					self.xman.add_ops(note, self.get_co(self.op_ga.get()), 
					[float(1.0 / (2.0 ** i)), ut.armonic_freq(int(self.axis_id_note.get()) + begin + (12 * i)), 0.0, 0.0, 10])
		self.update_lb_meth()
		self.axis_id_note.set(value='0')
		self.axis_tag_note.set(value='la')
		self.tag_axis_freq.set(value='Frecuencia 0.0 Hz')
		self.pres_gn.set(value='1')
		self.with_temp.set(value=False)
		self.op_ga.set(value='Seno')
		self.sec_win.destroy()
	def generate_armonics(self):
		self.xman.del_all_ops(self.curr_note.get())
		for x in self.fou.localize_armonics(float(self.value_flud.get())):
			self.xman.add_ops(self.curr_note.get(), self.get_co(self.op_ga.get()), [x[1], x[0], 0.0, 0.0, 10])
		self.update_lb_meth()
		self.update_cold_data()
		self.refresh_screen()
		self.sec_win.destroy()
		self.value_flud.set(value='0.25')
		self.op_ga.set(value='Seno')
	def identify_note(self):
		name_notes = self.occ_scale
		id_note = ut.find_id_note(self.fou.localize_fundamental())
		self.curr_note.set(name_notes[id_note])

	def switch_note_mode(self):
		self.xman.set_init(not self.mode_note.get())
		self.mode_note.set(not self.mode_note.get())

	def play_sound(self):
		self.fou.play_sound()
	def play_waves(self):
		self.update_cold_data()
		if len(self.cold_data):
			self.fou.play_wave(self.cold_data)

	#Layout de la App
	def create_vars(self): 
		# Operaciones Trigonométricas
		self.curr_expr = tk.StringVar(value='Operacion')
		self.value_a = tk.StringVar(value='1.0')
		self.value_b = tk.StringVar(value='1.0')
		self.value_c = tk.StringVar(value='0.0')
		self.value_d = tk.StringVar(value='0.0')
		self.value_e = tk.StringVar(value='10')
		#Simulacion de tiempo audio, simulación y espectro
		self.value_ab = tk.StringVar(value='0.0')
		self.value_at = tk.StringVar(value='1.0')
		self.value_sd = tk.StringVar(value='1.0')
		self.value_sb = tk.StringVar(value='0')
		self.value_st = tk.StringVar(value='50000')
		#Etiquetas Variables
		self.tag_a = tk.StringVar(value='Amplitud')
		self.tag_b = tk.StringVar(value='Frecuencia')
		self.tag_c = tk.StringVar(value='Fase')
		self.tag_d = tk.StringVar(value='Origen')
		self.tag_e = tk.StringVar(value='Precision')
		self.tag_bmn = tk.StringVar(value='Individual')
		#Valores de Plantilla
		self.curr_temp = tk.StringVar(value='None')
		self.value_att = tk.StringVar(value='0.2')
		self.value_dec = tk.StringVar(value='0.2')
		self.value_sus = tk.StringVar(value='0.4')
		self.value_rel = tk.StringVar(value='0.2')
		self.value_pic = tk.StringVar(value='1.0')
		self.value_stb = tk.StringVar(value='0.6')
		self.value_res = tk.StringVar(value='0.4')
		#Operadores de Generación XML
		self.curr_note = tk.StringVar(value='all')
		self.value_id = tk.StringVar(value='0')
		self.mode_note = tk.BooleanVar(value=True)
		#Panel de Generacion de Armónicos
		self.value_flud = tk.StringVar(value='0.25')
		self.op_ga = tk.StringVar(value='Seno')
		#Panel de Generacion de Notas
		self.axis_id_note = tk.StringVar(value='0')
		self.axis_tag_note = tk.StringVar(value='la')
		self.tag_axis_freq = tk.StringVar(value='Frecuencia 0.0 Hz')
		self.pres_gn = tk.StringVar(value='1')
		self.with_temp = tk.BooleanVar(value=False)
		#Valiables de Dibujo
		self.show_audio = tk.BooleanVar(value=True)
		self.show_hot = tk.BooleanVar(value=True)
		self.show_cold = tk.BooleanVar(value=False)
		self.show_temp = tk.BooleanVar(value=False)
		self.show_four = tk.BooleanVar(value=False)
		#Variables Metadatos
		self.value_name = tk.StringVar(value='Instrumento')
		self.value_vol = tk.StringVar(value='0.8')
		self.value_durM = tk.StringVar(value='3.0')
		
		self.curr_note.trace('w',self.update_lb_meth)
		self.curr_expr.trace('w',self.expr_listener)
		self.mode_note.trace('w',self.update_xml_format)
		self.axis_id_note.trace('w',self.get_axis_id_freq)

		self.value_a.trace('w',self.val_meth_list)
		self.value_b.trace('w',self.val_meth_list)
		self.value_c.trace('w',self.val_meth_list)
		self.value_d.trace('w',self.val_meth_list)
		self.value_e.trace('w',self.val_meth_list)

		self.value_ab.trace('w',self.val_meth_list)
		self.value_at.trace('w',self.val_meth_list)
		self.value_sd.trace('w',self.val_meth_list)
		self.value_sb.trace('w',self.val_meth_list)
		self.value_st.trace('w',self.val_meth_list)

		self.curr_temp.trace('w',self.val_temp_list)
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
		self.nt_expression = ttk.Notebook(self.master, height=280, width=850)
		self.nt_control = ttk.Notebook(self.master, height=280, width=850)

		self.fr_method = tk.Frame(self.nt_expression, height=280, width=850)
		self.fr_method.pack(expand=True)
		self.fr_template = tk.Frame(self.nt_expression, height=280, width=850)
		self.fr_template.pack(expand=True)
		self.fr_xml = tk.Frame(self.nt_control,height=280, width=850)
		self.fr_xml.pack(expand= True)
		self.fr_draw = tk.Frame(self.nt_control,height=280, width=850)
		self.fr_draw.pack(expand= True)
		self.fr_audio = tk.Frame(self.nt_control,height=280, width=850)
		self.fr_audio.pack(expand= True)
		
		self.nt_expression.add(self.fr_template, text = "Plantilla")
		self.nt_expression.add(self.fr_method, text = "Método")
		self.nt_expression.grid(column=0, row=1, sticky='NW')

		self.nt_control.add(self.fr_xml, text = "XML")
		self.nt_control.add(self.fr_draw, text = "Gráfica")
		self.nt_control.add(self.fr_audio, text = "Audio")
		self.nt_control.grid(column=1, row=1, sticky='NW')

		#Canvas
		self.cv_main.get_tk_widget().config(height=750, width=1900)
		self.cv_main.get_tk_widget().grid(column=0, row=0, columnspan= 2,sticky='N')
		
		#Guardado de Archivo XML
		self.btn_safe = tk.Button(self.fr_xml, text="Exportar", command=self.save_xml_file, width=25)
		self.btn_safe.grid(column=0, row=0, sticky='N')
		self.btn_load = tk.Button(self.fr_xml, text="Cargar", command=self.load_xml_file, width=25)#width=110
		self.btn_load.grid(column=0, row=1, sticky='N')
		self.btn_noise = tk.Button(self.fr_xml, text="Probar Sonido", command=self.play_waves, width=25)#width=110
		self.btn_noise.grid(column=0, row=2, sticky='N')
		self.btn_keyb = tk.Button(self.fr_xml, text="Probar en Teclado", width=25)#width=110
		self.btn_keyb.grid(column=0, row=3, sticky='N')
		self.btn_swc_mn = tk.Button(self.fr_xml, textvariable=self.tag_bmn, command=self.switch_note_mode, width=25)
		self.btn_swc_mn.grid(column=1, row=0, sticky='N')
		self.btn_agn = tk.Button(self.fr_xml, text="Rellenar Notas", command=self.ask_for_cf_params, width=25)#width=110
		self.btn_agn.grid(column=1, row=1, sticky='N')
		self.lbl_name = tk.Label(self.fr_xml, text='Nombre del Instrumento')
		self.lbl_name.grid(column=3,row=0,sticky='N')
		self.txb_name = tk.Entry(self.fr_xml, textvariable=self.value_name)
		self.txb_name.grid(column=2,row=0,sticky='N')
		self.lbl_vol = tk.Label(self.fr_xml, text='Volumen')
		self.lbl_vol.grid(column=3,row=1,sticky='N')
		self.txb_vol = tk.Entry(self.fr_xml, textvariable=self.value_vol)
		self.txb_vol.grid(column=2,row=1,sticky='N')
		self.lbl_durM = tk.Label(self.fr_xml, text='Duración MAX de Nota')
		self.lbl_durM.grid(column=3,row=2,sticky='N')
		self.txb_durM = tk.Entry(self.fr_xml, textvariable=self.value_durM)
		self.txb_durM.grid(column=2,row=2,sticky='N')
		#Opciones del Lienzo
		self.chb_hot = tk.Checkbutton(self.fr_draw, text="Actual", fg='red', variable=self.show_hot, onvalue=True, offvalue=False)
		self.chb_hot.grid(column = 0, row = 0, sticky='N')
		self.chb_cold = tk.Checkbutton(self.fr_draw, text="Modelo", fg='blue', variable=self.show_cold, onvalue=True, offvalue=False)
		self.chb_cold.grid(column = 0, row = 1, sticky='N')
		self.chb_audio = tk.Checkbutton(self.fr_draw, text="Audio", fg='gold', variable=self.show_audio, onvalue=True, offvalue=False)
		self.chb_audio.grid(column = 0, row = 2,sticky='N')
		self.chb_cold = tk.Checkbutton(self.fr_draw, text="Plantilla", fg='orange', variable=self.show_temp, onvalue=True, offvalue=False)
		self.chb_cold.grid(column = 1, row = 0, sticky='N')
		self.chb_four = tk.Checkbutton(self.fr_draw, text="Fourier", fg='purple', variable=self.show_four, onvalue=True, offvalue=False)
		self.chb_four.grid(column = 1, row = 1,sticky='N')
		#Opciones de audio
		self.btn_audio = tk.Button(self.fr_audio, text="Cargar", command=self.load_wav_file, width=50)# width=115
		self.btn_audio.grid(column = 0, row = 0, columnspan=2,sticky='N')
		self.btn_audio = tk.Button(self.fr_audio, text="Reproducir", command=self.play_sound, state=tk.DISABLED, width=50)# width=115
		self.btn_audio.grid(column = 0, row = 1, columnspan=2,sticky='N')
		self.lbl_ab = tk.Label(self.fr_audio, text='Inicio Audio')
		self.lbl_ab.grid(column=0, row=2,sticky='N')
		self.txb_ab = tk.Entry(self.fr_audio, bd=4, textvariable=self.value_ab, state=tk.DISABLED)#width=55
		self.txb_ab.grid(column=0, row=3,sticky='N')
		self.lbl_at = tk.Label(self.fr_audio, text='Tiempo Audio')
		self.lbl_at.grid(column=1, row=2,sticky='N')
		self.txb_at = tk.Entry(self.fr_audio, bd=4, textvariable=self.value_at, state=tk.DISABLED)#width=55
		self.txb_at.grid(column=1, row=3,sticky='N')
		self.lbl_sb = tk.Label(self.fr_audio, text='Inicio Espectro')
		self.lbl_sb.grid(column=0, row=4,sticky='N')
		self.txb_sb = tk.Entry(self.fr_audio, bd=4, textvariable=self.value_sb, state=tk.DISABLED)#width=55
		self.txb_sb.grid(column=0, row=5,sticky='N')
		self.lbl_st = tk.Label(self.fr_audio, text='Longitud Espectro')
		self.lbl_st.grid(column=1, row=4,sticky='N')
		self.txb_st = tk.Entry(self.fr_audio, bd=4, textvariable=self.value_st, state=tk.DISABLED)#width=55
		self.txb_st.grid(column=1, row=5,sticky='N')

		#Panel de plantilla
		self.cbx_type_temp = ttk.Combobox(self.fr_template,values=["None","ADSR"], textvariable = self.curr_temp)
		self.cbx_type_temp.set("None")
		self.cbx_type_temp.grid(columnspan=4,sticky='N')

		self.lbl_attack = tk.Label(self.fr_template, text = 'Ascenso')
		self.lbl_attack.grid(column=0,row=1)
		self.txb_attack = tk.Entry(self.fr_template, bd=4, textvariable = self.value_att)
		self.txb_attack.grid(column=0,row=2)

		self.lbl_decay = tk.Label(self.fr_template, text = 'Caída')
		self.lbl_decay.grid(column=1,row=1)
		self.txb_decay = tk.Entry(self.fr_template, bd=4, textvariable = self.value_dec)
		self.txb_decay.grid(column=1,row=2)

		self.lbl_sustain = tk.Label(self.fr_template, text = 'Sostén')
		self.lbl_sustain.grid(column=2,row=1)
		self.txb_sustain = tk.Entry(self.fr_template, bd=4, textvariable = self.value_sus)
		self.txb_sustain.grid(column=2,row=2)

		self.lbl_release = tk.Label(self.fr_template, text = 'Liberar')
		self.lbl_release.grid(column=3,row=1)
		self.txb_release = tk.Entry(self.fr_template, bd=4, textvariable = self.value_rel)
		self.txb_release.grid(column=3,row=2)

		self.lbl_pitch = tk.Label(self.fr_template, text = 'Pico')
		self.lbl_pitch.grid(column=0,row=3)
		self.txb_pitch = tk.Entry(self.fr_template, bd=4, textvariable = self.value_pic)
		self.txb_pitch.grid(column=0,row=4)

		self.lbl_stable = tk.Label(self.fr_template, text = 'Estable')
		self.lbl_stable.grid(column=1,row=3)
		self.txb_stable = tk.Entry(self.fr_template, bd=4, textvariable = self.value_stb)
		self.txb_stable.grid(column=1,row=4)

		self.lbl_rest = tk.Label(self.fr_template, text = 'Descenso')
		self.lbl_rest.grid(column=2,row=3)
		self.txb_rest = tk.Entry(self.fr_template, bd=4, textvariable = self.value_res)
		self.txb_rest.grid(column=2,row=4)

		#Panel de metodos
		self.btn_add_sum = tk.Button(self.fr_method, text="Añadir", command=self.set_method)
		self.btn_add_sum.grid(column=1,row=0, sticky='W')
		self.btn_mod_sum = tk.Button(self.fr_method, text="Editar", command=self.mod_method)
		self.btn_mod_sum.grid(column=2,row=0, sticky='W')
		self.btn_rmv_sum = tk.Button(self.fr_method, text="Quitar", command=self.del_method)
		self.btn_rmv_sum.grid(column=3,row=0, sticky='W')
		self.btn_gnr_arm = tk.Button(self.fr_method, text="Auto-Armónicos", command=self.ask_for_ga_params, state=tk.DISABLED)
		self.btn_gnr_arm.grid(column=4,row=0, sticky='W')

		self.sb_sum = tk.Scrollbar(self.fr_method, orient='vertical')
		self.sb_sum.grid(column=0,row=1,rowspan=5,sticky='NS')
		self.lb_sum = tk.Listbox(self.fr_method, yscrollcommand = self.sb_sum.set) #,height=175, width=240)
		self.lb_sum.grid(column=1,row=1,columnspan=4, rowspan=5, sticky='WE')
		self.sb_sum.config(command=self.lb_sum.yview)

		self.cbx_ops = ttk.Combobox(self.fr_method,values=["Seno","Cuadratica","Triangular","SierraAna","SierraOpt","Ruido"],textvariable=self.curr_expr)
		self.cbx_ops.set("Operacion")
		self.cbx_ops.grid(column=5,row=0, columnspan=2, sticky='N')

		self.cbx_note = ttk.Combobox(self.fr_method,values=self.occ_scale, textvariable=self.curr_note)
		self.cbx_note.config(state=tk.DISABLED)
		self.cbx_note.grid(column=7, row=3, sticky='E')

		self.lbl_a = tk.Label(self.fr_method, textvariable=self.tag_a)
		self.lbl_a.grid(column=6,row=1,sticky='N')
		self.txb_a = tk.Entry(self.fr_method, bd=4, textvariable=self.value_a)
		self.txb_a.grid(column=5,row=1,sticky='N')
		self.lbl_b = tk.Label(self.fr_method, textvariable=self.tag_b)
		self.lbl_b.grid(column=6,row=2,sticky='N')
		self.txb_b = tk.Entry(self.fr_method, bd=4, textvariable=self.value_b)
		self.txb_b.grid(column=5,row=2,sticky='N')
		self.lbl_c = tk.Label(self.fr_method, textvariable=self.tag_c)
		self.lbl_c.grid(column=6,row=3,sticky='N')
		self.txb_c = tk.Entry(self.fr_method, bd=4, textvariable=self.value_c)
		self.txb_c.grid(column=5,row=3,sticky='N')
		self.lbl_d = tk.Label(self.fr_method, textvariable=self.tag_d)
		self.lbl_d.grid(column=6,row=4,sticky='N')
		self.txb_d = tk.Entry(self.fr_method, bd=4, textvariable=self.value_d)
		self.txb_d.grid(column=5,row=4,sticky='N')
		self.lbl_e = tk.Label(self.fr_method, textvariable=self.tag_e)
		self.lbl_e.grid(column=6,row=5,sticky='N')
		self.txb_e = tk.Entry(self.fr_method, bd=4, textvariable=self.value_e, state=tk.DISABLED)
		self.txb_e.grid(column=5,row=5,sticky='N')

		self.btn_id = tk.Button(self.fr_method, text='ID Armonic', command = self.get_armonic_freq)
		self.btn_id.grid(column=7,row=1,sticky='N')
		self.txb_id = tk.Entry(self.fr_method, bd=4, textvariable=self.value_id)
		self.txb_id.grid(column=7,row=2,sticky='N')

		self.lbl_dur = tk.Label(self.fr_method, text = 'Duración')
		self.lbl_dur.grid(column=8,row=1)
		self.txb_dur = tk.Entry(self.fr_method, bd=4, textvariable = self.value_sd)
		self.txb_dur.grid(column=8,row=2)

root = tk.Tk()

def close():
	plt.close('all')
	root.destroy()

def main():
	root.wm_title("Tuner by JoGEHrt V_0.9.3")
	root.wm_protocol('WM_DELETE_WINDOW',close)
	app = Application(root)
	app.mainloop()

if __name__ == "__main__":
	main()