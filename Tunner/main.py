import tkinter as tk
from tkinter import ttk
from tkinter import filedialog
from tkinter.filedialog import askopenfile

from lib.osc import Oscillador as osc
from lib.xml_man import XML_Manager

class Application(tk.Frame):
	def __init__(self, master=None):
		super().__init__(master)
		self.master = master
		self.cv_main = tk.Canvas(self.master)
		self.txb_e = tk.Entry(self.master)
		self.filename = '../instrumento.xml'
		self.xman = XML_Manager(self.filename)

		self.x_sim = 2160
		self.y_sim = 1000
		self.center = int(self.y_sim / 2)
		self.px_unit = 180
		self.zoom_val = 1.0
		self.iv = [1.0, 1.0, 0.0, 0.0, 20.0]

		self.hot_data = []
		self.hot_data.extend(range(0,self.x_sim))
		osc.clear_array(self.hot_data)

		self.warm_data = []
		self.warm_data.extend(range(0,self.x_sim))
		osc.clear_array(self.warm_data)

		self.aux_data = []
		self.aux_data.extend(range(0,self.x_sim))
		osc.clear_array(self.aux_data)

		self.cold_data = []
		self.cold_data.extend(range(0,self.x_sim))
		osc.clear_array(self.cold_data)

		self.create_vars()
		self.create_widgets()
		self.draw_scale()
		self.master.state('zoomed') # "1920x980"
		
	#Métodos gráficos
	def draw_scale(self):
		for i in range(0, self.y_sim + 1, osc.round(10 * self.zoom_val)):#Strips X
				self.cv_main.create_line(0, i, self.x_sim, i, width=1, fill='ivory2')
		pcount=0
		for i in range(0,self.x_sim + 1, osc.round(10 * self.zoom_val)):#Strips Y
			if(i % osc.round(180 * self.zoom_val)):#Regulares
				self.cv_main.create_line(i, 0, i, self.y_sim, width=1, fill='ivory2')
			else:#Cada Pi/2
				self.cv_main.create_line(i, 0, i, self.y_sim, width=1, fill='gray')
				if(pcount%2):#Si el contador es impar
					self.cv_main.create_text(i + 5,self.center + 10,text="{pc}/2Pi".format(pc=pcount),fill='black',font=('Arial 12 bold'))
				elif(pcount>0):
					self.cv_main.create_text(i + 5,self.center + 10,text="{pc}Pi".format(pc=int(pcount/2)),fill='black',font=('Arial 12 bold'))
				pcount+=1
		pcount=0
		for i in range(0, self.center + 1, osc.round(self.px_unit * self.zoom_val)):#Strips X
			if(i):
				self.cv_main.create_line(0, self.center + i, self.x_sim, self.center + i, width=1, fill='gray')
				self.cv_main.create_text(15, self.center + i + 10, text="{pc}".format(pc=-pcount),fill='black',font=('Arial 12 bold'))
			self.cv_main.create_line(0, self.center - i, self.x_sim, self.center - i, width=1, fill='gray')
			self.cv_main.create_text(5, self.center - i+10, text="{pc}".format(pc=pcount),fill='black',font=('Arial 12 bold'))
			pcount+=1
		self.cv_main.create_line(0, self.center, self.x_sim, self.center, width=2, fill='black')	#X
		self.cv_main.create_line(0,0,0,self.y_sim,width=2,fill='black')								#Y
		self.draw_disc_dot(self.x_sim,self.y_sim)
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
		self.cv_main.create_rectangle(0, 0, self.x_sim, self.y_sim,fill='white',outline='white')
	#Dibujo de Funciones
	def draw_data(self, values, org = 0, color='red'):
		scl = self.px_unit * self.zoom_val
		if(self.count_opt.get()):
			last_res = self.center - (osc.round(values[org] * scl ))
			for i in range(org,len(values),1):
				res = self.center - (osc.round(values[i] * scl ))
				last_res = self.draw_cont_dot(i, res, last_res, color)
		else:
			for i in range(org,len(values),1):
				res = self.center - (osc.round(values[i] * scl ))
				self.draw_disc_dot(i, res, color)
	def update_hot_data(self):
		osc.clear_array(self.hot_data)
		input_data = [self.curr_expr.get(), self.iv[0], self.iv[1], self.iv[2], self.iv[3], 0, self.x_sim, self.zoom_val, int(self.iv[4])]
		osc.operation(input_data, self.hot_data)
	def update_warm_data(self):
		osc.clear_array(self.warm_data)
		if(len(self.xman.read_all_ops('la','method'))):
			for i in self.xman.read_all_ops('la','method'):
				method_data = [i[0], float(i[1]), float(i[2]), float(i[3]), float(i[4]), 0, self.x_sim, self.zoom_val, int(i[5])]
				osc.operation(method_data, self.warm_data)

		osc.operation(self.warm_data, self.hot_data)

		if(len(self.xman.read_all_ops('la','template'))):
			for i in self.xman.read_all_ops('la','template'):
				template_data = [i[0], float(i[1]), float(i[2]), float(i[3]), float(i[4]), 0, self.x_sim, self.zoom_val, int(i[5])]
				osc.operation(template_data, self.aux_data)
			osc.op_arrays(self.warm_data, self.aux_data, False)
	def update_cold_data(self):
		osc.clear_array(self.cold_data)
		osc.clear_array(self.aux_data)
		if(len(self.xman.read_all_ops('la','method'))):
			for i in self.xman.read_all_ops('la','method'):
				method_data = [i[0], float(i[1]), float(i[2]), float(i[3]), float(i[4]), 0, self.x_sim, self.zoom_val, int(i[5])]
				osc.operation(method_data, self.cold_data)

		if(len(self.xman.read_all_ops('la','template'))):
			for i in self.xman.read_all_ops('la','template'):
				template_data = [i[0], float(i[1]), float(i[2]), float(i[3]), float(i[4]), 0, self.x_sim, self.zoom_val, int(i[5])]
				osc.operation(template_data, self.aux_data)
			osc.op_arrays(self.cold_data, self.aux_data, False)
		
	def refresh_screen(self,*args):
		self.clear_canvas()
		self.draw_scale()
		if(self.show_cold.get() and len(self.cold_data) > 0):
			self.draw_data(values = self.cold_data, color = 'cyan')
		if(self.show_warm.get() and len(self.warm_data) > 0):
			self.draw_data(values = self.warm_data, color = 'yellow')
		if(self.show_hot.get() and len(self.hot_data) > 0):
			self.draw_data(values = self.hot_data, color = 'red')
	#Controladores
	def entry_listener(self,*args):
		self.iv = [1.0, 1.0, 0.0, 0.0, 20.0]
		self.txb_e.config(state=tk.DISABLED)
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

		if(self.curr_expr.get() == "SierraAna"):
			self.txb_e.config(state=tk.NORMAL)
		self.update_hot_data()
		self.update_warm_data()
		self.refresh_screen()

	def set_ops_sum(self):
		self.lb_sum.insert(self.lb_sum.size(),
		'{:.2f}{}({:.2f}w + {:.2f}) +{:.2f}'.format(
		float(self.value_a.get()), self.curr_expr.get(), float(self.value_b.get()), float(self.value_c.get()), float(self.value_d.get()) ))
		self.xman.add_ops('la', 'method', self.curr_expr.get(), self.value_a.get(), self.value_b.get(), self.value_c.get(), self.value_d.get(), self.value_e.get())
		self.update_cold_data()
		self.refresh_screen()
	def del_ops_sum(self):
		i = self.lb_sum.curselection()[0]
		self.xman.del_ops('la', 'method', i)
		self.lb_sum.delete(self.lb_sum.curselection())
		self.update_cold_data()
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
		self.update_cold_data()
		self.refresh_screen()

	def set_ops_prod(self):
		self.lb_prod.insert(self.lb_prod.size(),
		'{:.2f}{}({:.2f}w + {:.2f}) +{:.2f}'.format(
		float(self.value_a.get()), self.curr_expr.get(), float(self.value_b.get()), float(self.value_c.get()), float(self.value_d.get()) ))
		self.xman.add_ops('la', 'template', self.curr_expr.get(), self.value_a.get(), self.value_b.get(), self.value_c.get(), self.value_d.get(), self.value_e.get())
		self.update_cold_data()
		self.refresh_screen()	
	def del_ops_prod(self):
		i = self.lb_sum.curselection()[0]
		self.xman.del_ops('la', 'template', i)
		self.lb_prod.delete(self.lb_prod.curselection())
		self.update_cold_data()
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
		self.update_cold_data()
		self.refresh_screen()

	def save_file(self):
		filename = filedialog.asksaveasfilename(initialdir='./export',
        filetypes=[("XML data", ".xml")])
		#print(filename)
		if filename:
			if filename.find('.xml') == -1:
				filename += '.xml'
			self.xman.set_path(filename)
			self.xman.bake()
	def load_file(self):
		filename = filedialog.askopenfilename()
		#print(filename)
		self.xman.load_archive(filename)
		self.lb_prod.delete(0, tk.END)
		self.lb_sum.delete(0, tk.END)
		if filename:
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
			self.update_cold_data()
			self.refresh_screen()

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
		self.show_hot = tk.BooleanVar(value=True)
		self.show_warm = tk.BooleanVar(value=True)
		self.show_cold = tk.BooleanVar(value=True)

		self.curr_expr.trace('w',self.entry_listener)
		self.value_a.trace('w',self.entry_listener)
		self.value_b.trace('w',self.entry_listener)
		self.value_c.trace('w',self.entry_listener)
		self.value_d.trace('w',self.entry_listener)
		self.value_e.trace('w',self.entry_listener)
		self.value_z.trace('w',self.entry_listener)

		self.count_opt.trace('w',self.refresh_screen)
		self.show_hot.trace('w',self.refresh_screen)
		self.show_warm.trace('w',self.refresh_screen)
		self.show_cold.trace('w',self.refresh_screen)
	def create_widgets(self):
		#Frames
		self.fr_canvas = tk.Frame(self.master)
		self.fr_canvas.place(x=10,y=20, height=550, width=1510)
		self.fr_expression = tk.Frame(self.master)
		self.fr_expression.place(x=10,y=575, height=195, width=850)
		self.fr_control = tk.Frame(self.master)
		self.fr_control.place(x=865,y=575, height=195, width=660)
		
		#Panel de Control del Canvas
		self.btn_create = tk.Button(self.fr_control, text="Crear")
		self.btn_create.place(y=25,width=110)
		self.btn_safe = tk.Button(self.fr_control, text="Guardar", command=self.save_file)
		self.btn_safe.place(y=50,width=110)
		self.btn_load = tk.Button(self.fr_control, text="Cargar", command=self.load_file)
		self.btn_load.place(y=75,width=110)
		self.btn_delete = tk.Button(self.fr_control, text="Eliminar")
		self.btn_delete.place(y=100,width=110)
		self.lbl_z = tk.Label(self.fr_expression, text="Zoom")
		self.lbl_z.place(x=115,y=10, width=95)
		self.txb_zoom = tk.Entry(self.fr_control, bd=4, textvariable=self.value_z)
		self.txb_zoom.place(x=115,y=25, width=95)
		self.chb_disc = tk.Checkbutton(self.fr_control, text="Discreto", variable=self.count_opt, onvalue=True, offvalue=False)
		self.chb_disc.place(x=115,y=50)
		self.chb_hot = tk.Checkbutton(self.fr_control, text="Op Inmediata", fg='red', variable=self.show_hot, onvalue=True, offvalue=False)
		self.chb_hot.select()
		self.chb_hot.place(x=115,y=70)
		self.chb_warm = tk.Checkbutton(self.fr_control, text="Op Posible", fg='yellow', variable=self.show_warm, onvalue=True, offvalue=False)
		self.chb_hot.select()
		self.chb_warm.place(x=115,y=90)
		self.chb_cold = tk.Checkbutton(self.fr_control, text="Op Total", fg='cyan', variable=self.show_cold, onvalue=True, offvalue=False)
		self.chb_hot.select()
		self.chb_cold.place(x=115,y=110)

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

		self.options = ["Seno","Cuadratica","Triangular","SierraOpt","SierraAna","Ruido","Segmento"]
		self.cbx_ops = ttk.Combobox(self.fr_expression,values=self.options,textvariable=self.curr_expr)
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
		self.cv_main = tk.Canvas(self.fr_canvas, bg="white", height=530, width=1510)
		self.clear_canvas()
		self.cv_main.grid(row=0, column=1)
		self.scroll_x = tk.Scrollbar(self.fr_canvas, orient="horizontal", command=self.cv_main.xview)
		self.scroll_x.grid(row=1, column=1, sticky="ew")
		self.scroll_y = tk.Scrollbar(self.fr_canvas, orient="vertical", command=self.cv_main.yview)
		self.scroll_y.grid(row=0, column=0, sticky="ns")

		self.cv_main.configure(yscrollcommand=self.scroll_y.set, xscrollcommand=self.scroll_x.set)
		self.cv_main.configure(scrollregion=self.cv_main.bbox("all"))

root = tk.Tk()
root.wm_title("Tuner by JoGEHrt V_0.2.2")
app = Application(root)
app.mainloop()