import xml.etree.ElementTree as ET

class XML_Manager():
	def __init__(self, path):
		self.path = path
		self.set_init()

	def load_archive(self, path):
		self.path = path
		self.root = ET.parse(self.path).getroot()

	def set_init(self, is_gen = True):
		if is_gen:
			self.root = ET.Element('instrument')
			note = ET.SubElement(self.root, 'note',{'from': 'all'})
			ET.SubElement(note,'template',{'type':'None'})
			ET.SubElement(note,'method')
		else:
			self.root = ET.Element('instrument')
			notes = []
			for tag in ['la','la#','si','do','do#','re','re#','mi','fa','fa#','sol','sol#']:
				notes.append(ET.SubElement(self.root, 'note',{'from': tag}))
			for note in notes:
				ET.SubElement(note,'template',{'type':'None'})
				ET.SubElement(note,'method')
	def set_template(self, tag, type_t, val):
		subject = self.root.find("./note[@from='{}']/template[@type='{}']/amplitude".format(tag, type_t))
		if subject != None:
			subject.set('att','{}'.format(val[0]))
			subject.set('dec','{}'.format(val[1]))
			subject.set('sus','{}'.format(val[2]))
			subject.set('rel','{}'.format(val[3]))
			subject.set('pic','{}'.format(val[4]))
			subject.set('stb','{}'.format(val[5]))
			subject.set('res','{}'.format(val[6]))
			return 0
		subject =  self.root.find("./note[@from='{}']/template".format(tag))
		if subject != None:
			self.root.find("./note[@from='{}']".format(tag)).remove(subject)
		temp = ET.SubElement(self.root.find("./note[@from='{}']".format(tag)),'template',{'type':'{}'.format(type_t)})
		ET.SubElement(temp, 'amplitude',{'att':'{}'.format(val[0]),
										 'dec':'{}'.format(val[1]),
										 'sus':'{}'.format(val[2]),
										 'rel':'{}'.format(val[3]),
										 'pic':'{}'.format(val[4]),
										 'stb':'{}'.format(val[5]),
										 'res':'{}'.format(val[6])})
		return 0
		
	def add_ops(self, tag, op_type, val):
		subject = self.root.find("./note[@from='{}']/method".format(tag))
		ET.SubElement(subject, 'operation',{'type':'{}'.format(op_type),
											'amp':'{}'.format(val[0]),
											'frq':'{}'.format(val[1]),
											'vam':'{}'.format(val[2]),
											'vfq':'{}'.format(val[3]),
											'det':'{}'.format(val[4])})
	def mod_ops(self, tag, index, op_type, val):
		subject = self.root.findall("./note[@from='{}']/method/".format(tag))[index]
		subject.set('type','{}'.format(op_type))
		subject.set('amp','{}'.format(val[0]))
		subject.set('frq','{}'.format(val[1]))
		subject.set('vam','{}'.format(val[2]))
		subject.set('vfq','{}'.format(val[3]))
		subject.set('det','{}'.format(val[4]))
	def del_ops(self, tag, index):
		subject = self.root.findall("./note[@from='{}']/method/".format(tag))[index]
		self.root.find("./note[@from='{}']/method".format(tag)).remove(subject)
		
	def del_all_ops(self, tag):
		for sub in self.root.findall("./note[@from='{}']/method/".format(tag)):
			self.root.find("./note[@from='{}']/method".format(tag)).remove(sub)

	def read_ops(self, tag, index):
		sub = self.root.findall("./note[@from='{}']/method/".format(tag))[index]
		return [sub.get('type'),sub.get('amp'),sub.get('frq'),sub.get('vam'),sub.get('vfq'),sub.get('det')]
	def read_temp(self, tag):
		type_t = self.root.find("./note[@from='{}']/template".format(tag))
		if(type_t.get('type') == 'None'):
			return ['None','0.0','0.0','0.0','0.0','0.0','0.0','0.0']
		sub = self.root.find("./note[@from='{}']/template/amplitude".format(tag))
		return [type_t.get('type'), sub.get('att'),sub.get('dec'),sub.get('sus'),sub.get('rel'),sub.get('pic'),sub.get('stb'),sub.get('res')]
	def read_all_ops(self, tag):
		xml_data = []
		for sub in self.root.findall("./note[@from='{}']/method/".format(tag)):
			xml_data.append([sub.get('type'),sub.get('amp'),sub.get('frq'),sub.get('vam'),sub.get('vfq'),sub.get('det')])
		return xml_data

	def get_size_ops(self, tag):
		return len(self.root.findall("./note[@from='{}']/method/".format(tag)))
	def get_mode(self):
		return self.root.find("./note[@from='all']") != None

	def bake(self):
		ET.indent(self.root)
		ET.ElementTree(self.root).write(self.path, xml_declaration=True, encoding='utf8')
	def get_path(self):
		return self.path
	def set_path(self,newpath):
		self.path = newpath
'''
xman = XML_Manager('file.xml')
xman.bake()
xman.set_path('file2.xml')

xman.set_template('all','ADSR',[1.0,2.0,3.0,4.0,5.0,6.0,7.0])
xman.bake()
xman.set_path('file3.xml')

xman.set_init(False)
xman.set_template('la','ADSR',[1.0,2.0,3.0,4.0,5.0,6.0,7.0])
xman.set_template('do#','ADSR',[1.0,2.0,3.0,4.0,5.0,6.0,7.0])
xman.set_template('do#','ADSR',[7.0,6.0,5.0,4.0,3.0,2.0,1.0])
xman.bake()
'''