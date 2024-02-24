import xml.etree.ElementTree as ET

class XML_Manager():
	def __init__(self, path):
		self.path = path
		self.root = ET.Element('instrument')
		self.notes = []
		for tag in ['la','si','do','re','mi','fa','sol']:
			self.notes.append(ET.SubElement(self.root, 'note',{'catholic': tag}))
		for note in self.notes:
			temp = ET.SubElement(note,'template',{'type':'ADSR'})
			ET.SubElement(temp,'amplitude',{'att':'0.2',
											'dec':'0.2',
											'sus':'0.4',
											'rel':'0.2',
											'pic':'1.0',
											'stb':'0.6',
											'res':'0.4'})	
			ET.SubElement(note,'method')

	def load_archive(self, path):
		self.path = path
		self.root = ET.parse(self.path).getroot()

	def set_template(self, tag, val):
		subject = self.root.find("./note[@catholic='{}']/template/amplitude".format(tag))
		subject.set('att','{}'.format(val[0]))
		subject.set('dec','{}'.format(val[1]))
		subject.set('sus','{}'.format(val[2]))
		subject.set('rel','{}'.format(val[3]))
		subject.set('pic','{}'.format(val[4]))
		subject.set('stb','{}'.format(val[5]))
		subject.set('res','{}'.format(val[6]))	
	def add_ops(self, tag, op_type, val):
		subject = self.root.find("./note[@catholic='{}']/method".format(tag))
		ET.SubElement(subject, 'operation',{'type':'{}'.format(op_type),
											'amp':'{}'.format(val[0]),
											'frq':'{}'.format(val[1]),
											'fse':'{}'.format(val[2]),
											'org':'{}'.format(val[3]),
											'det':'{}'.format(val[4])})
	def mod_ops(self, tag, index, op_type, val):
		subject = self.root.findall("./note[@catholic='{}']/method/".format(tag))[index]
		subject.set('type','{}'.format(op_type))
		subject.set('amp','{}'.format(val[0]))
		subject.set('frq','{}'.format(val[1]))
		subject.set('fse','{}'.format(val[2]))
		subject.set('org','{}'.format(val[3]))
		subject.set('det','{}'.format(val[4]))
	def del_ops(self, tag, index):
		subject = self.root.findall("./note[@catholic='{}']/method/".format(tag))[index]
		self.root.find("./note[@catholic='{}']/method".format(tag)).remove(subject)

	def read_ops(self, tag, index):
		sub = self.root.findall("./note[@catholic='{}']/method/".format(tag))[index]
		return [sub.get('type'),sub.get('amp'),sub.get('frq'),sub.get('fse'),sub.get('org'),sub.get('det')]
	def read_temp(self, tag):
		sub = self.root.find("./note[@catholic='{}']/template/amplitude".format(tag))
		return [sub.get('att'),sub.get('dec'),sub.get('sus'),sub.get('rel'),sub.get('pic'),sub.get('stb'),sub.get('res')]
	def read_all_ops(self, tag):
		xml_data = []
		for sub in self.root.findall("./note[@catholic='{}']/method/".format(tag)):
			xml_data.append([sub.get('type'),sub.get('amp'),sub.get('frq'),sub.get('fse'),sub.get('org'),sub.get('det')])
		return xml_data

	def get_size_ops(self, tag):
		return len(self.root.findall("./note[@catholic='{}']/method/".format(tag)))

	def bake(self):
		ET.indent(self.root)
		ET.ElementTree(self.root).write(self.path, xml_declaration=True, encoding='utf8')
	def get_path(self):
		return self.path
	def set_path(self,newpath):
		self.path = newpath
