import tkinter as tk
from tkinter import ttk
from tkinter import filedialog
from tkinter.filedialog import askopenfile

from lib.osc import Oscillador as osc
from lib.util_math import Util as ut
from lib.osc import Operations as ops 
from lib.xml_man import XML_Manager
from lib.audio_imp import AudioFile

class Application(tk.Frame):
	def __init__(self, master=None):
		super().__init__(master)
		self.master = master
		self.cv_main = tk.Canvas(self.master)
		self.txb_e = tk.Entry(self.master)
		self.scroll_x = tk.Scrollbar(self.master)
		self.scroll_y = tk.Scrollbar(self.master)
		self.file_inst = '../instrumento.xml'
		self.file_audio = '../sonido.wav'
		self.xman = XML_Manager(self.file_inst)
		self.af = AudioFile()

		#Valores de simulacion
		self.x_sim = 1490
		self.y_sim = 530
		self.center = int(self.y_sim / 2)
		self.px_unit = 180
		self.px_time = 180
		self.zoom_val = 1.0
		self.dx_scroll = 0.0
		self.dy_scroll = 0.5

		self.iv = [1.0, 1.0, 0.0, 0.0, 20]
		self.action_val = True

		self.hot_data = []
		self.warm_data = []
		self.aux_data = []
		self.cold_data = []
		self.audio_data = []
		self.aux_aud_data = []

		self.create_vars()
		self.create_widgets()
		self.draw_scale()
		self.master.state('zoomed') # "1920x980"
	#Métodos gráficos
	def get_unit(self, value, case):
		if(case == 'R4'):
			if(value % 2):
				return '{}Pi'.format(int(value/2))
			return '{}Pi/2'.format(value)
		if(case == 'S'):
			if(value > 1.0):
				return '{:.1f}s'.format(value)
			return '{}s'.format(int(value))
		if(case == 'A'):
			return '{}'.format(int(value))
	def draw_scale(self):
		dx = ut.round_i(1000 * self.dx_scroll * self.zoom_val) 
		dy = ut.round_i(500 * (self.dy_scroll - 0.5) * self.zoom_val)
		for i in range(0, self.y_sim + 1, 10):		#Horizontal Strips
			y_pos = ut.surround_i(i - dy, self.y_sim)
			self.cv_main.create_line(0, y_pos, self.x_sim, y_pos, width=1, fill='ivory2')
		
		n = 1 / self.zoom_val
		for i in range(0, self.x_sim + 1, 10):	#Vertical Strips
			x_pos = ut.surround_i(i - dx, self.x_sim)
			self.cv_main.create_line(x_pos, 0, x_pos, self.y_sim, width=1, fill='ivory2')#Regulares
			if(i % self.px_time == 0 and i != 0):#Cada unidad marcada
				self.cv_main.create_line(x_pos, 0, x_pos, self.y_sim, width=1, fill='gray')
				self.cv_main.create_text(x_pos + 5, self.center + 10 - dy, text = self.get_unit(n,'S'), fill='black',font=('Arial 12 bold'))
				n += 1 / self.zoom_val
		
		n = 0
		for i in range(0, self.center + 1, self.px_unit):#Print Values
			y_pos = ut.surround_i(i + dy, self.y_sim)
			y_npos = ut.surround_i(i - dy, self.y_sim)
			if(i):#Skip Cero
				self.cv_main.create_line(0, self.center + y_npos, self.x_sim, self.center + y_npos, width=1, fill='gray')
				self.cv_main.create_text(15, self.center + y_npos + 10, text = self.get_unit(-n,'A'),fill='black',font=('Arial 12 bold'))
			self.cv_main.create_line(0, self.center - y_pos, self.x_sim, self.center - y_pos, width=1, fill='gray')
			self.cv_main.create_text(15, self.center - y_pos + 10, text = self.get_unit(n,'A'),fill='black',font=('Arial 12 bold'))
			n += 1 / self.zoom_val


		self.cv_main.create_line(0, self.center - dy, self.x_sim, self.center - dy, width=2, fill='black')	#Axis X
		self.cv_main.create_line(5, 0, 5, self.y_sim, width=2, fill='black')						#Axis Y
	def draw_cont_dot(self, x:int, y:int, y0:int, color ='red')->int:
		if(y > y0):
			y0 += 1
		elif(y < y0):
			y0 -= 1#create_rectangle(x, y, x, y) Graficar un punto
		self.cv_main.create_rectangle(x, y0, x, y,width=1, outline=color)
		return y
	def draw_disc_dot(self, x:int, y:int, color ='red'):
		self.cv_main.create_rectangle(x, y, x, y,width=1, outline=color)
	def clear_canvas(self):
		self.cv_main.create_rectangle(0, 0, self.x_sim + 1, self.y_sim + 1,fill='white',outline='white')	
	#Dibujo de Funciones
	def draw_data(self, values, org = 0, color='red'):
		scl = self.px_unit * self.zoom_val
		dy = ut.round_i(500 * (self.dy_scroll - 0.5) * self.zoom_val)
		if(self.count_opt.get()):
			last_res = self.center - (ut.round_i(values[0] * scl)) - dy
			for i in range(0, len(values)):
				res = self.center - (ut.round_i(values[i] * scl)) - dy
				if(res <= self.y_sim and res >= 0):
					last_res = self.draw_cont_dot(i + 5, res, last_res, color)
				else:
					last_res = res
		else:
			for i in range(0, len(values)):
				res = self.center - (ut.round_i(values[i] * scl )) - dy
				if(res <= self.y_sim and res >= 0):
					self.draw_disc_dot(i + 5, res, color)
	def update_hot_data(self):
		dx = ut.round_i(1000 * self.dx_scroll * self.zoom_val)
		input_data = [[ops[self.curr_expr.get()].value, self.iv[0], self.iv[1], self.iv[2], self.iv[3], self.iv[4]]]
		self.hot_data = osc.operation(input_data, dx, self.x_sim, self.zoom_val)
	def update_warm_data(self):
		dx = ut.round_i(1000 * self.dx_scroll * self.zoom_val)
		method_data = []
		template_data = []
		method_ops = self.xman.read_all_ops('la','method')
		template_ops = self.xman.read_all_ops('la','template')
		if(len(method_ops)):#Si existe contenido por sumar
			for i in method_ops:
				method_data.append([ops[i[0]].value, float(i[1]), float(i[2]), float(i[3]), float(i[4]), int(i[5])])
			self.warm_data = osc.operation(method_data, dx, self.x_sim, self.zoom_val)
			osc.op_arrays(self.warm_data, self.hot_data)

		if(len(template_ops)):#Si existe contenido por
			for i in template_ops:
				template_data.append([ops[i[0]].value, float(i[1]), float(i[2]), float(i[3]), float(i[4]), int(i[5])])
			self.aux_data = osc.operation(template_data, dx, self.x_sim, self.zoom_val)
			if(len(method_ops)):
				osc.op_arrays(self.warm_data, self.aux_data, False)
			else:
				self.warm_data = self.aux_data
				osc.op_arrays(self.warm_data, self.hot_data)
	def update_cold_data(self):
		dx = ut.round_i(1000 * self.dx_scroll * self.zoom_val)
		method_data = []
		template_data = []
		method_ops = self.xman.read_all_ops('la','method')
		template_ops = self.xman.read_all_ops('la','template')
		if(len(method_ops)):#Si existe contenido por sumar
			for i in method_ops:
				method_data.append([ops[i[0]].value, float(i[1]), float(i[2]), float(i[3]), float(i[4]), int(i[5])])
			self.cold_data = osc.operation(method_data, dx, self.x_sim, self.zoom_val)

		if(len(template_ops)):#Si existe contenido por
			for i in template_ops:
				template_data.append([ops[i[0]].value, float(i[1]), float(i[2]), float(i[3]), float(i[4]), int(i[5])])
			self.aux_data = osc.operation(template_data, dx, self.x_sim, self.zoom_val)
			if(len(method_ops)):
				osc.op_arrays(self.cold_data, self.aux_data, False)
			else:
				self.cold_data = self.aux_data
	def update_audio_data(self):
		self.aux_aud_data = []
		factor = 30.625
		for i in range(0,self.x_sim):
			if(ut.round_i(i * factor) < len(self.audio_data)):
				self.aux_aud_data.append(self.audio_data[ut.round_i(i * factor)] /5000)
			else:
				pass

	def refresh_screen(self,*args):
		self.clear_canvas()
		self.draw_scale()
		if(self.show_audio.get() and len(self.aux_aud_data) > 0):
			self.draw_data(values = self.aux_aud_data, color = 'green')
		if(self.show_cold.get() and len(self.cold_data) > 0):
			self.draw_data(values = self.cold_data, color = 'cyan')
		if(self.show_warm.get() and len(self.warm_data) > 0):
			self.draw_data(values = self.warm_data, color = 'yellow')
		if(self.show_hot.get() and len(self.hot_data) > 0):
			self.draw_data(values = self.hot_data, color = 'red')

	#Controladores
	def value_listener(self, *args):
		if(self.action_val):
			self.iv = [1.0, 1.0, 0.0, 0.0, 20.0]
			self.zoom_val = 1.0
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
				self.iv[4] = float(self.value_e.get())
			except ValueError:
				pass
			try:
				self.zoom_val = float(self.value_z.get())
				if(self.zoom_val <= 0.0):
					self.zoom_val = 1.0
			except ValueError:
				pass
			if(self.show_hot.get() and len(self.hot_data) > 0):
				self.update_hot_data()
			if(self.show_warm.get() and len(self.warm_data) > 0):
				self.update_warm_data()
			self.refresh_screen()
	def expr_listener(self,*args):
		self.action_val = False
		self.iv = [1.0, 1.0, 0.0, 0.0, 0.0]
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
			self.iv = [1.0, 1.0, 0.0, 0.0, 20.0]
			self.tag_e.set(value='Precision')
			self.value_e.set(value='20')
		if(self.curr_expr.get() == "SierraOpt"):
			self.value_b.set(value='0.005')
			self.iv = [1.0, 0.005, 0.0, 0.0, 0.0]
		if(self.curr_expr.get() == "Ruido"):
			self.tag_a.set(value='Minimo')
			self.tag_b.set(value='Maximo')
			self.tag_c.set(value='Origen')
			self.tag_d.set(value='Semilla')
			self.value_a.set(value='-1.0')
			self.value_b.set(value='1.0')
			self.iv = [-1.0, 1.0, 0.0, 0.0, 0.0]
		if(self.curr_expr.get() == "Segmento"):
			self.tag_a.set(value='Pendiente')
			self.tag_b.set(value='Origen')
			self.tag_c.set(value='Inicio')
			self.tag_d.set(value='Final')
			self.value_a.set(value='1.0')
			self.value_b.set(value='0.0')
			self.value_c.set(value='0.0')
			self.value_d.set(value='100.0')
			self.iv = [1.0, 0.0, 0.0, 100.0, 0.0]
		if(self.curr_expr.get() == "SenoArmonico"):
			self.value_b.set(value='64')
			self.iv = [1.0, 64.0, 0.0, 0.0, 0.0]
		self.action_val = True

		if(self.show_hot.get() and len(self.hot_data) > 0):
			self.update_hot_data()
		if(self.show_warm.get() and len(self.warm_data) > 0):
			self.update_warm_data()
		self.refresh_screen()

	def set_ops_sum(self):
		self.lb_sum.insert(self.lb_sum.size(),
		'{:.2f}{}({:.2f}w + {:.2f}) +{:.2f}'.format(
		float(self.value_a.get()), self.curr_expr.get(), float(self.value_b.get()), float(self.value_c.get()), float(self.value_d.get()) ))
		self.xman.add_ops('la', 'method', self.curr_expr.get(), self.value_a.get(), self.value_b.get(), self.value_c.get(), self.value_d.get(), self.value_e.get())
		
		if(self.show_cold.get() and len(self.cold_data) > 0):
			self.update_cold_data()
		if(self.show_warm.get() and len(self.warm_data) > 0):
			self.update_warm_data()
		self.refresh_screen()
	def del_ops_sum(self):
		i = self.lb_sum.curselection()[0]
		self.xman.del_ops('la', 'method', i)
		self.lb_sum.delete(self.lb_sum.curselection())
		if(self.show_cold.get() and len(self.cold_data) > 0):
			self.update_cold_data()
		if(self.show_warm.get() and len(self.warm_data) > 0):
			self.update_warm_data()
		self.refresh_screen()	
	def mod_ops_sum(self):
		a = self.lb_sum.curselection()[0]
		xml_data = self.xman.read_ops('la', 'method', a)
		self.curr_expr.set(xml_data[0])
		self.value_a.set(xml_data[1])
		self.value_b.set(xml_data[2])
		self.value_c.set(xml_data[3])
		self.value_d.set(xml_data[4])
		self.value_e.set(xml_data[5])
		self.lb_sum.delete(a)
		if(self.show_cold.get() and len(self.cold_data) > 0):
			self.update_cold_data()
		if(self.show_warm.get() and len(self.warm_data) > 0):
			self.update_warm_data()
		self.refresh_screen()

	def set_ops_prod(self):
		self.lb_prod.insert(self.lb_prod.size(),
		'{:.2f}{}({:.2f}w + {:.2f}) +{:.2f}'.format(
		float(self.value_a.get()), self.curr_expr.get(), float(self.value_b.get()), float(self.value_c.get()), float(self.value_d.get()) ))
		self.xman.add_ops('la', 'template', self.curr_expr.get(), self.value_a.get(), self.value_b.get(), self.value_c.get(), self.value_d.get(), self.value_e.get())
		if(self.show_cold.get() and len(self.cold_data) > 0):
			self.update_cold_data()
		if(self.show_warm.get() and len(self.warm_data) > 0):
			self.update_warm_data()
		self.refresh_screen()	
	def del_ops_prod(self):
		i = self.lb_sum.curselection()[0]
		self.xman.del_ops('la', 'template', i)
		self.lb_prod.delete(self.lb_prod.curselection())
		if(self.show_cold.get() and len(self.cold_data) > 0):
			self.update_cold_data()
		if(self.show_warm.get() and len(self.warm_data) > 0):
			self.update_warm_data()
		self.refresh_screen()
	def mod_ops_prod(self):
		a = self.lb_prod.curselection()[0]
		xml_data = self.xman.read_ops('la', 'template', a)
		self.curr_expr.set(xml_data[0])
		self.value_a.set(xml_data[1])
		self.value_b.set(xml_data[2])
		self.value_c.set(xml_data[3])
		self.value_d.set(xml_data[4])
		self.value_e.set(xml_data[5])
		self.lb_prod.delete(a)
		if(self.show_cold.get() and len(self.cold_data) > 0):
			self.update_cold_data()
		if(self.show_warm.get() and len(self.warm_data) > 0):
			self.update_warm_data()
		self.refresh_screen()

	def save_xml_file(self):
		file_inst = filedialog.asksaveasfile_inst(initialdir='./export', filetypes=[("XML data", ".xml")])
		#print(file_inst)
		if file_inst:
			if file_inst.find('.xml') == -1:
				file_inst += '.xml'
			self.xman.set_path(file_inst)
			self.xman.bake()
	def load_xml_file(self):
		file_inst = filedialog.askopenfile_inst(initialdir='./export', filetypes=[("XML data", ".xml")])
		if file_inst:
			self.xman.load_archive(file_inst)
			self.lb_prod.delete(0, tk.END)
			self.lb_sum.delete(0, tk.END)
			data = self.xman.read_all_ops('la','template')
			for x in data:
				self.lb_prod.insert(self.lb_prod.size(),
				'{:.2f}{}({:.2f}w + {:.2f}) +{:.2f}'.format(
				float(x[1]), x[0], float(x[2]), float(x[3]), float(x[4])))

			data = self.xman.read_all_ops('la','method')
			for x in data:
				self.lb_sum.insert(self.lb_sum.size(),
				'{:.2f}{}({:.2f}w + {:.2f}) +{:.2f}'.format(
				float(x[1]), x[0], float(x[2]), float(x[3]), float(x[4])))
			if(self.show_cold.get() and len(self.cold_data) > 0):
				self.update_cold_data()
			if(self.show_warm.get() and len(self.warm_data) > 0):
				self.update_warm_data()
			self.refresh_screen()

	def load_wav_file(self):
		path = filedialog.askopenfile(initialdir='./samples', filetypes=[("WAV audio", ".wav")])
		if path.name:
			self.af.load_archive(path.name)
			self.audio_data = self.af.get_slice_at_time(time=1.0)
			self.update_audio_data()

	def set_x_sim(self, a, b):
		if(	abs(float(b) - self.dx_scroll) >= 0.01):
			self.x_begin.set(float(b))
			if(self.show_cold.get() and len(self.cold_data) > 0):
				self.update_cold_data()
			self.dx_scroll = abs(ut.round_f(float(b), 2))
		self.scroll_x.set(float(b), float(b) + 0.01)
	def set_y_sim(self, a, b):
		if(	abs(float(b) - self.dy_scroll) >= 0.01):
			self.y_begin.set(float(b) - 0.5)
			if(self.show_cold.get() and len(self.cold_data) > 0):
				self.update_cold_data()
			self.dy_scroll = abs(ut.round_f(float(b), 2))
		self.scroll_y.set(float(b), float(b) + 0.01)

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
		self.tag_a = tk.StringVar(value='Amplitud')
		self.tag_b = tk.StringVar(value='Frecuencia')
		self.tag_c = tk.StringVar(value='Fase')
		self.tag_d = tk.StringVar(value='Origen')
		self.tag_e = tk.StringVar(value='Precision')

		self.count_opt = tk.BooleanVar(value=True)
		self.show_audio = tk.BooleanVar(value=True)
		self.show_hot = tk.BooleanVar(value=True)
		self.show_warm = tk.BooleanVar(value=True)
		self.show_cold = tk.BooleanVar(value=True)

		self.x_begin = tk.DoubleVar(value=0.0)
		self.y_begin = tk.DoubleVar(value=0.0)

		self.curr_expr.trace('w',self.expr_listener)

		self.value_a.trace('w',self.value_listener)
		self.value_b.trace('w',self.value_listener)
		self.value_c.trace('w',self.value_listener)
		self.value_d.trace('w',self.value_listener)
		self.value_e.trace('w',self.value_listener)
		self.value_z.trace('w',self.value_listener)

		self.count_opt.trace('w',self.refresh_screen)
		self.show_hot.trace('w',self.refresh_screen)
		self.show_warm.trace('w',self.refresh_screen)
		self.show_cold.trace('w',self.refresh_screen)

		self.x_begin.trace('w',self.value_listener)
		self.y_begin.trace('w',self.value_listener)
	def create_widgets(self):
		#Frames
		self.fr_canvas = tk.Frame(self.master)
		self.fr_canvas.place(x=10,y=20, height=550, width=1510)
		self.fr_expression = tk.Frame(self.master)
		self.fr_expression.place(x=10,y=575, height=195, width=850)
		self.fr_control = tk.Frame(self.master)
		self.fr_control.place(x=865,y=575, height=195, width=660)
		
		#Panel de Control del Canvas
		self.btn_safe = tk.Button(self.fr_control, text="Guardar", command=self.save_xml_file)
		self.btn_safe.place(y=25,width=110)
		self.btn_load = tk.Button(self.fr_control, text="Cargar", command=self.load_xml_file)
		self.btn_load.place(y=50,width=110)
		self.btn_audio = tk.Button(self.fr_control, text="Audio", command=self.load_wav_file)
		self.btn_audio.place(y=75,width=110)
		self.lbl_z = tk.Label(self.fr_control, text="Zoom")
		self.lbl_z.place(x=115,y=10, width=95)
		self.cbx_zoom = ttk.Combobox(self.fr_control, values=["10","5","2","1","0.5","0.2","0.1"], textvariable=self.value_z)
		self.cbx_zoom.set("1")
		self.cbx_zoom.place(x=115, y=27, width=95)
		self.chb_disc = tk.Checkbutton(self.fr_control, text="Discreto", variable=self.count_opt, onvalue=True, offvalue=False)
		self.chb_disc.place(x=115,y=50)
		self.chb_hot = tk.Checkbutton(self.fr_control, text="Op Inmediata", fg='red', variable=self.show_hot, onvalue=True, offvalue=False)
		self.chb_hot.select()
		self.chb_hot.place(x=115,y=70)
		self.chb_warm = tk.Checkbutton(self.fr_control, text="Op Posible", fg='yellow', variable=self.show_warm, onvalue=True, offvalue=False)
		self.chb_warm.select()
		self.chb_warm.place(x=115,y=90)
		self.chb_cold = tk.Checkbutton(self.fr_control, text="Op Total", fg='cyan', variable=self.show_cold, onvalue=True, offvalue=False)
		self.chb_cold.select()
		self.chb_cold.place(x=115,y=110)
		self.chb_audio = tk.Checkbutton(self.fr_control, text="Audio", fg='green', variable=self.show_audio, onvalue=True, offvalue=False)
		self.chb_audio.select()
		self.chb_audio.place(x=115,y=130)

		#Panel de Generacion de Expresiones
		self.txb_a = tk.Entry(self.fr_expression, bd=4, textvariable=self.value_a)
		self.txb_a.place(x=215,y=35, width=95)
		self.txb_b = tk.Entry(self.fr_expression, bd=4, textvariable=self.value_b)
		self.txb_b.place(x=315,y=35, width=95)
		self.txb_c = tk.Entry(self.fr_expression, bd=4, textvariable=self.value_c)
		self.txb_c.place(x=415,y=35, width=95)
		self.txb_d = tk.Entry(self.fr_expression, bd=4, textvariable=self.value_d)
		self.txb_d.place(x=515,y=35, width=95)
		self.txb_e = tk.Entry(self.fr_expression, bd=4, textvariable=self.value_e, state=tk.DISABLED)
		self.txb_e.place(x=615,y=35, width=95)

		self.lbl_a = tk.Label(self.fr_expression, textvariable=self.tag_a)
		self.lbl_a.place(x=215,y=10, width=95)
		self.lbl_b = tk.Label(self.fr_expression, textvariable=self.tag_b)
		self.lbl_b.place(x=315,y=10, width=95)
		self.lbl_c = tk.Label(self.fr_expression, textvariable=self.tag_c)
		self.lbl_c.place(x=415,y=10, width=95)
		self.lbl_d = tk.Label(self.fr_expression, textvariable=self.tag_d)
		self.lbl_d.place(x=515,y=10, width=95)
		self.lbl_e = tk.Label(self.fr_expression, textvariable=self.tag_e)
		self.lbl_e.place(x=615,y=10, width=95)

		self.cbx_ops = ttk.Combobox(self.fr_expression,values=["Seno","Cuadratica","Triangular","SierraAna","SierraOpt","Ruido","Segmento","SenoArmonico"],textvariable=self.curr_expr)
		self.cbx_ops.set("Operacion")
		self.cbx_ops.place(x=10, y=35, width=200)

		#Muestra de salida
		self.lbl_prod = tk.Label(self.fr_expression, text="Producto")
		self.lbl_prod.place(x=10,y=75)
		self.btn_add_prod = tk.Button(self.fr_expression, text="Añadir", command=self.set_ops_prod)
		self.btn_add_prod.place(x=165,y=75,width=85)
		self.btn_mod_prod = tk.Button(self.fr_expression, text="Editar", command=self.mod_ops_prod)
		self.btn_mod_prod.place(x=250,y=75,width=85)
		self.btn_rmv_prod = tk.Button(self.fr_expression, text="Quitar", command=self.del_ops_prod)
		self.btn_rmv_prod.place(x=335,y=75,width=85)

		self.lbl_prod = tk.Label(self.fr_expression, text="Suma")
		self.lbl_prod.place(x=430,y=75)
		self.btn_add_sum = tk.Button(self.fr_expression, text="Añadir", command=self.set_ops_sum)
		self.btn_add_sum.place(x=585,y=75,width=85)
		self.btn_mod_sum = tk.Button(self.fr_expression, text="Editar", command=self.mod_ops_sum)
		self.btn_mod_sum.place(x=670,y=75,width=85)
		self.btn_rmv_sum = tk.Button(self.fr_expression, text="Quitar", command=self.del_ops_sum)
		self.btn_rmv_sum.place(x=755,y=75,width=85)
		
		self.sb_prod = tk.Scrollbar(self.fr_expression)
		self.sb_prod.place(x=10, y=105, height=80)
		self.lb_prod = tk.Listbox(self.fr_expression, yscrollcommand = self.sb_prod.set)
		self.lb_prod.place(x=25,y=105,height=80,width=395)
		self.sb_prod.config(command=self.lb_prod.yview)

		self.sb_sum = tk.Scrollbar(self.fr_expression)
		self.sb_sum.place(x=430, y=105, height=80)
		self.lb_sum = tk.Listbox(self.fr_expression, yscrollcommand = self.sb_sum.set)
		self.lb_sum.place(x=445,y=105,height=80,width=395)
		self.sb_sum.config(command=self.lb_sum.yview)
		
		#Panel del Canvas
		self.cv_main = tk.Canvas(self.fr_canvas, bg="white", height=530, width=1490)
		self.clear_canvas()
		self.cv_main.grid(row=0, column=0)
		self.scroll_x = tk.Scrollbar(self.fr_canvas, orient="horizontal", command = self.set_x_sim)
		self.scroll_x.grid(row=1, column=0, sticky="ew")
		self.scroll_y = tk.Scrollbar(self.fr_canvas, orient="vertical", command = self.set_y_sim)
		self.scroll_y.set(0.495, 0.505)
		self.scroll_y.grid(row=0, column=1, sticky="ns")

		#self.cv_main.configure(yscrollcommand=self.scroll_y.set, xscrollcommand=self.scroll_x.set)
		#self.cv_main.configure(scrollregion=self.cv_main.bbox("all"))

def main():
	root = tk.Tk()
	root.wm_title("Tuner by JoGEHrt V_0.3.0")
	app = Application(root)
	app.mainloop()

if __name__ == "__main__":
	main()