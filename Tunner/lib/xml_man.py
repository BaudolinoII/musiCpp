import xml.etree.ElementTree as ET

class XML_Manager():
	def __init__(self, path):
		self.path = path
		self.root = ET.Element('instrument')
		self.notes = []
		for tag in ['la','si','do','re','mi','fa','sol']:
			self.notes.append(ET.SubElement(self.root, 'note',{'catholic': tag}))
		for note in self.notes:
			ET.SubElement(note,'template',{'type':'ADSR'})
			ET.SubElement(note,'method')

	def load_archive(self, path):
		self.path = path
		self.root = ET.parse(self.path).getroot()
	
	def add_ops(self, tag, block, op_type, val1, val2, val3, val4='0.0', val5='0'):
		subject = self.root.find("./note[@catholic='{}']/{}".format(tag,block))
		ET.SubElement(subject, 'operation',{'type':'{}'.format(op_type),
											'amp':'{}'.format(val1),
											'freq':'{}'.format(val2),
											'fase':'{}'.format(val3),
											'org':'{}'.format(val4),
											'det':'{}'.format(val5)})
	def mod_ops(self, tag, block, index, op_type, val1, val2, val3, val4='0.0', val5='0'):
		subject = self.root.findall("./note[@catholic='{}']/{}/".format(tag,block))[index]
		subject.set('type','{}'.format(op_type))
		subject.set('amp','{}'.format(val1))
		subject.set('freq','{}'.format(val2))
		subject.set('fase','{}'.format(val3))
		subject.set('org','{}'.format(val4))
		subject.set('det','{}'.format(val5))
	def del_ops(self, tag, block, index):
		subject = self.root.findall("./note[@catholic='{}']/{}/".format(tag,block))[index]
		self.root.find("./note[@catholic='{}']/{}".format(tag,block)).remove(subject)

	def read_ops(self, tag, block, index):
		sub = self.root.findall("./note[@catholic='{}']/{}/".format(tag,block))[index]
		return [sub.get('type'),sub.get('amp'),sub.get('freq'),sub.get('fase'),sub.get('org'),sub.get('det')]
	def read_all_ops(self, tag, block):
		xml_data = []
		for sub in self.root.findall("./note[@catholic='{}']/{}/".format(tag,block)):
			xml_data.append([sub.get('type'),sub.get('amp'),sub.get('freq'),sub.get('fase'),sub.get('org'),sub.get('det')])
		return xml_data

	def bake(self):
		ET.indent(self.root)
		ET.ElementTree(self.root).write(self.path, xml_declaration=True, encoding='utf8')
	def get_path(self):
		return self.path
	def set_path(self,newpath):
		self.path = newpath
'''
mg = XML_Manager('ejemplo_avanzado.xml')
mg.add_ops('la','method','sin','1.0','1.0','0.0','0.0')
mg.add_ops('la','method','sin','1.0','1.0','0.0','0.0')
mg.add_ops('la','method','sin','1.0','1.0','0.0','0.0')
mg.mod_ops('la','method', 1, 'sin','2.0','2.0','1.0','1.0')
mg.del_ops('la','method', 2)
data = mg.read_ops('la','method', 0)
print(data)
mg.bake()
'''