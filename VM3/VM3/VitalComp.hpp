#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <stdexcept>
#include <regex>
#include <vector>
#include <stack>

#ifndef BIT8
#define BIT8 unsigned char
#endif

const std::regex regex_commentBlock("(//[^\\n]*)");
const std::regex regex_commentLine("(/\\*[^*]*\\*/)");
const std::regex regex_space("[^\\w\\d\\S]+");
const std::regex regex_import("(IMPORT\\s+'([^']+)';)");
const std::regex regex_path("'([^']+)'");
const std::regex regex_varDec("VAR([^;]*);");
const std::regex regex_inst("INST([^;]*);");
const std::regex regex_args("\\s*(\\b\\w+\\b)\\s*");
const std::regex regex_prop("([\\w]+)\\s*=\\s*'([^']*)'");
const std::regex regex_group("GROUP\\s+(\\w+)\\s*\\{([^}]*)\\};");
const std::regex regex_script("SCRIPT\\s+(\\w+)\\s*\\{([^}]*)\\};");
const std::regex regex_mscript("MAIN_SCRIPT\\s*\\{([^}]*)\\};");
const std::regex regex_melody("(MELODY\\s*\\<([^>]+)\\>\"([^\"]+)\"(\\([^)]*\\))?;)");
const std::regex regex_event("(PLAY|PAUSE|STOP)\\s*\\<([^>]+)\\>;");

const std::vector<std::string> occ_scale12 = { "La", "La#", "Si", "Do", "Do#", "Re", "Re#", "Mi", "Fa", "Fa#", "Sol", "Sol#" };
const std::vector<std::string> chdCms = { "13M","13","11M","11","9M","9","#9","B9","7M","7","/6","6","MAY","MAJ","MIN","MEN","AUM","AUG","DIS","DIM","SUS","5","#5","B5","4","3","2" };

const std::vector<std::string> ins_play = { "id", "type", "start", "loop", "scale"};
const std::vector<std::string> ins_pause = { "id", "start", "duration" };
const std::vector<std::string> ins_stop = { "id", "at" };

const std::string HEX = "0123456789ABCDEF";
const std::string splitMark = " \t\n\r";
const std::string splitCap = ",;:";
const char C0 = -48, C1 = -36, C2 = -24, C3 = -12, C4 = 0, C5 = 12, C6 = 24, C7 = 36;
const BIT8 RN = 0x80, BL = 0x40, NG = 0x20, CH = 0x10, SH = 0x08, FS = 0x04, SF = 0x02, QF = 0x01;

bool existElement(const std::string str, const std::string arg) {
	if (!arg.size())
		return false;
	size_t size_arg = arg.size(), i = str.find_first_of(arg[0]);
	while (i != std::string::npos && (i + size_arg) <= str.size()) {
		if (!str.substr(i, size_arg).compare(arg))
			return true;
		i = str.find_first_of(arg[0], i + 1);
	}
	return false;
}
bool existAnyof(const std::string str, const std::string args) {
	return str.find(args) != std::string::npos;
}
bool existAllof(const std::string str, const std::string args) {
	for (size_t i = 0; i < args.size(); i++)
		if (str.find(args[i]) == std::string::npos)
			return false;
	return true;
}
bool existElement(const std::string str, const char arg) {
	return str.find_first_of(arg) != std::string::npos;
}

int countElement(const std::string str, const char target) {
	int count = 0;
	size_t pos = str.find_first_of(target);
	while (pos != std::string::npos) {
		count++;
		pos = str.find_first_of(target, pos + 1);
	}
	return count;
}
int locateElement(const std::string str, const std::string args, const size_t cap) {
	if (!args.size())
		return -1;
	size_t size_arg = args.size(), i = str.find_first_of(args[0]), lim = (cap ? cap : str.size());
	while (i != std::string::npos && (i + size_arg) <= lim) {
		if (!str.substr(i, size_arg).compare(args))
			return i;
		i = str.find_first_of(args[0], i + 1);
	}
	return -1;
}

std::string getUpperCase(const std::string str) {
	std::string upp = "";
	for (size_t i = 0; i < str.size(); i++)
		if (str[i] > 0x60 && str[i] < 0x7A)
			upp.append(1, str[i] & 0xDF);
		else
			upp.append(1, str[i]);
	return upp;
}
std::string stractBlock(std::string& str, const std::string begin, const std::string end) {
	int pos_begin = locateElement(str, begin, 0);
	if (pos_begin == -1)
		return "NONE";
	int pos_end = locateElement(str, end, 0);
	if (pos_end == -1)
		return "NONE";
	std::string sub = str.substr(pos_begin + begin.size(), pos_end - pos_begin - (int)begin.size());
	str.erase(pos_begin, pos_end - pos_begin + end.size());
	return sub;
}

void clearString(std::string& str) {
	std::string clearText = std::regex_replace(str, regex_commentLine, " ");
	clearText = std::regex_replace(clearText, regex_commentBlock, " ");
	clearText = std::regex_replace(clearText, regex_space, " ");
	str = std::regex_replace(clearText, std::regex("^ +| +$|( ) +"), "$1");
}
void printArr(BIT8* arr) {
	for (size_t i = 0; arr[i] != 0x7F; i++)
		std::cout << "0x" << HEX[arr[i] >> 4] << HEX[arr[i] & 15] << ((i + 1) % 3 ? ", " : "\n");
}
void toUpperCase(std::string& str) {
	for (size_t i = 0; i < str.size(); i++)
		if (str[i] > 0x60 && str[i] < 0x7A)
			str[i] &= 0xDF;
}
void deleteElement(std::string& str, const std::string target) {
	size_t pos = str.find_first_of(target);
	while (pos != std::string::npos) {
		str.erase(pos, 1);
		pos = str.find_first_of(target);
	}
}
void deleteChars(std::string& str, const std::string target) {
	if (str.empty())
		return;
	int loc = locateElement(str, target, 0);
	if (loc == -1)
		return;
	str.erase(loc, target.size());
}
void replaceElements(std::vector<std::string>& vec_str, size_t& index, const std::vector<std::string> vec_tar) {
	vec_str.erase(vec_str.begin() + index);
	for (std::string tar : vec_tar)
		vec_str.insert(vec_str.begin() + index++, tar);
	index--;
}
void switchElements(std::vector<std::string>& vec_str, size_t a, size_t b) {
	std::string aux(vec_str[a]), bux(vec_str[b]);
	vec_str.erase(vec_str.begin() + a);
	vec_str.insert(vec_str.begin() + a, bux);
	vec_str.erase(vec_str.begin() + b);
	vec_str.insert(vec_str.begin() + b, aux);
}

size_t near2P(const size_t num) {
	size_t n = 0;
	while (num > (1 << n)) n++;
	return n;
}

BIT8* VtoA(std::vector<BIT8> vec) {
	if (vec.size() == 0)
		return nullptr;
	BIT8* arr = new BIT8[vec.size()];
	for (size_t i = 0; i < vec.size(); i++)
		arr[i] = vec[i];
	return arr;
}

namespace MelodyComp {
	std::vector<std::string> split(const std::string args) {
		std::vector<std::string> splits; std::string splitter = splitMark + splitCap;
		size_t pos = args.find_first_of(splitter), begin = 0;
		while (pos != std::string::npos) {
			if (pos - (int)begin) {//Si existe un argumento
				splits.push_back(args.substr(begin, pos - (int)begin));
				if (existElement(splitCap, args.at(pos))) {//Si es un split simbolico
					std::string s;
					s.append(1, args.at(pos));
					splits.push_back(s);
				}
			}
			begin = pos + 1;
			pos = args.find_first_of(splitter, begin);
		}
		splits.push_back(args.substr(begin, args.size() - (int)begin));
		return splits;
	}
	std::vector<std::string> deconst2P(const size_t num) {
		size_t z = num & 255, m = num >> 8, n = near2P(z) + 1;
		std::vector<std::string> vec;
		while (m) {
			vec.push_back("s_1");
			m--;
		}
		do {
			n--;
			if (z >= (1 << n)) {
				vec.push_back("s_" + std::to_string((int)128 >> n));
				z -= 1 << n;
			}
		} while (n);

		return vec;
	}
	std::string extractChordTime(std::string& closeMark, BIT8 defaultTime) {
		size_t pos = closeMark.find('>');
		std::string aux(closeMark.substr(pos + 1));
		closeMark.erase(pos + 1);
		if (aux.empty())
			return "_" + std::to_string((int)(128 / defaultTime));
		return aux;
	}

	BIT8 getValue(const std::string args){
		if (args[0] == 'C') {
			if (args[1] == '\0')
				return 0;
			BIT8 v = 0;
			try {
				v = std::stoi(args.substr(1));
			}
			catch (const std::invalid_argument& e) {
				v = 0;
				std::cout << "Excepción: cadena no contiene un número válido." << std::endl;
			}
			return 12 * (v - 4);
		}
		if (!args.compare("WHOLE"))
			return RN;
		if (!args.compare("HALF"))
			return BL;
		if (!args.compare("QUARTER"))
			return NG;
		if (!args.compare("EIGHT"))
			return CH;
		if (!args.compare("SIXTEEN"))
			return SH;
		if (!args.compare("FUSE"))
			return FS;
		if (!args.compare("HFUSE"))
			return SF;
		if (!args.compare("QFUSE"))
			return QF;
		BIT8 val = 0;
		try {
			val = std::stoi(args);
		} catch (const std::invalid_argument& e) {
			val = 0;
			std::cout << "Excepción: cadena no contiene un número válido." << std::endl;
		}
		return 0;
	}

	bool update_status(std::string arg, BIT8& status) {
		if (existElement(arg, '<')) {
			status |= 0x80;
			return true;
		}
		if (existElement(arg, '>')) {
			status &= 0x7F;
			return true;
		}
		return false;
	}
	BIT8 atomize_tone(std::string arg, char scale) {
		int adj = 12 * countElement(arg, '+') + countElement(arg, '#') - (countElement(arg, 'b') + 12 * countElement(arg, '-'));
		toUpperCase(arg);
		if (existElement(arg, "LA"))
			return 0x01 + scale + adj;
		if (existElement(arg, "SI"))
			return 0x03 + scale + adj;
		if (existElement(arg, "DO") || existElement(arg, "UT"))
			return 0x04 + scale + adj;
		if (existElement(arg, "RE"))
			return 0x06 + scale + adj;
		if (existElement(arg, "FA"))
			return 0x08 + scale + adj;
		if (existElement(arg, "SOL"))
			return 0x0A + scale + adj;
		if (existElement(arg, "MI"))
			return 0x0C + scale + adj;
		size_t pos = arg.find_first_of("SCDEFGHAB,");
		if (pos == std::string::npos)
			return 0;
		switch (arg[pos]) {
		case ',':
		case 'S': return 0x00;
		case 'A': return 0x01 + scale + adj;
		case 'B': case 'H': return 0x03 + scale + adj;
		case 'C': return 0x04 + scale + adj;
		case 'D': return 0x06 + scale + adj;
		case 'E': return 0x08 + scale + adj;
		case 'F': return 0x0A + scale + adj;
		case 'G': return 0x0C + scale + adj;
		default:  return 0x00;
		}
	}
	BIT8 atomize_time(std::string arg, BIT8 bUnit) {
		size_t pos = arg.find_first_of("_.");
		if (pos == std::string::npos)
			return bUnit;
		if ((pos + 1) == arg.size())
			return bUnit + (arg[pos] == '.' ? bUnit >> 1 : 0x00);
		size_t t = std::stoi(arg.substr(pos + 1, arg.size() - (int)pos - 1));
		if (t < 2) return RN + (arg[pos] == '.' ? BL : 0x00);
		if (t < 4) return BL + (arg[pos] == '.' ? NG : 0x00);
		if (t < 8) return NG + (arg[pos] == '.' ? CH : 0x00);
		if (t < 16) return CH + (arg[pos] == '.' ? SH : 0x00);
		if (t < 32) return SH + (arg[pos] == '.' ? FS : 0x00);
		if (t < 64) return FS + (arg[pos] == '.' ? SF : 0x00);
		if (t < 128) return SF + (arg[pos] == '.' ? QF : 0x00);
		return QF;
	}
	void makeChords(std::vector<std::string>& args, size_t& i, bool isNotOnChord) {
		int j = -1; size_t lim = (args[i].find("_.") == std::string::npos ? 0 : args[i].find("_.")), index = 0;
		std::string arg = "";
		for (std::string com : chdCms) {
			if (j = locateElement(args[i], com, lim) > -1) {
				arg = getUpperCase(args[i].substr(++j));
				deleteElement(arg, com);
				arg.erase(lim);
				break;
			}
			index++;

		}

		if (j == -1 || index > 26) return;
		char* chords = new char[8]{ -1,-1,-1,-1,-1,-1,-1,-1 };
		chords[0] = atomize_tone(args[i].substr(0, j), -1);
		std::string localTime = (lim ? args[i].substr(lim) : "");

		do {
			switch (index) {
			case 0: case 1: chords[7] = chords[0] + 21;// 13M | 13
				chords[3] = chords[0] + 10 + (index == 1 ? 0 : 1);
			case 2: case 3: chords[6] = chords[0] + 17;// 11M | 11
				if (chords[3] == -1) chords[3] = chords[0] + 10 + (index == 3 ? 0 : 1);
			case 4: case 5: chords[5] = chords[0] + 14; // 9M | 9
				if (chords[3] == -1) chords[3] = chords[0] + 10 + (index == 5 ? 0 : 1);
			case 6: if (chords[5] == -1) chords[5] = chords[0] + 13;//  B9
			case 7: if (chords[5] == -1) chords[5] = chords[0] + 15;//  #9
			case 8: case 9: if (chords[3] == -1) chords[3] = chords[0] + 10 + (index == 9 ? 0 : 1);//  7M | 7
			case 11: if (index == 11 || index == 10) chords[4] = chords[0] + 9; // 6
			case 10: if (index == 10) chords[3] = chords[0] + 10; // /6
			case 12: case 13: chords[2] = chords[0] + 7; chords[1] = chords[0] + 4; break;// MAY | MAJ
			case 14: case 15: chords[2] = chords[0] + 7; chords[1] = chords[0] + 3; break;// MEN | MIN
			case 16: case 17: chords[2] = chords[0] + 8; chords[1] = chords[0] + 4; break;// AUM | AUG
			case 18: case 19: chords[2] = chords[0] + 6; chords[1] = chords[0] + 3; break;// DIS | DIM
			case 20: chords[2] = chords[0] + 7; chords[1] = chords[0] + 5; break;// SUS
			case 21: chords[2] = chords[0] + 7; break;//  5
			case 22: chords[2] = chords[0] + 6; break;//  B5
			case 23: chords[2] = chords[0] + 8; break;//  #5
			case 24: chords[1] = chords[0] + 5; break;//  4
			case 25: chords[1] = chords[0] + 4; break;//  3
			case 26: chords[1] = chords[0] + 2; break;//  2
			default: arg.clear();
			} index = 0;
			for (std::string com : chdCms) {
				if (j = locateElement(args[i], com, lim) > -1 || arg.empty()) {
					deleteElement(arg, com);
					break;
				} index++;
			}
		} while (!arg.empty() && index < 27);

		std::vector<std::string> commChord;
		for (size_t j = 0; j < 8; j++) if (chords[j] > -1) {
			std::string chr = "";
			chr.append(chords[j] / 12, '+');
			chr.append(occ_scale12[chords[j] % 12]);
			if (!localTime.empty()) chr.append(localTime);
			commChord.push_back(chr);
		}
		if (isNotOnChord) {
			commChord.insert(commChord.begin(), "<");
			commChord.insert(commChord.end() - 1, ">");
		}
		replaceElements(args, i, commChord);
	}
	void unpackChords(std::vector<std::string>& args, size_t nBeats, BIT8 bUnit) {
		size_t biggest = 0, begin = 0;
		BIT8 lrChd = bUnit;
		int accmTime = 0;
		const size_t mbpC = nBeats * bUnit;
		bool activeChord = false;
		std::string localTime = "";
		for (size_t i = 0; i < args.size(); i++) {
			std::string arg = getUpperCase(args[i]);
			if (existAllof(args[i], "<>")) {//En caso de contener un solo elemento
				if ((countElement(args[i], '_') + countElement(args[i], '.')) == 2)//en caso de tener 2 tiempos
					args[i].erase(args[i].find('>'));//Se queda con el propio del acorde
				deleteChars(args[i], "<>");//Se retiran los símbolos
			}
			if (existElement(args[i], '<')) {
				std::string aux(args[i]);
				deleteChars(aux, "<");
				replaceElements(args, i, { "<", aux });
				size_t j = i; while (!existElement(args[j], '>') && j < args.size()) j++;
				localTime = extractChordTime(args[j], bUnit);
				activeChord = true; biggest = i; begin = i; lrChd = bUnit;
			}
			if (activeChord) {
				if (!existAnyof(args[i], "_.")) args[i].append(localTime);
				BIT8 possible = atomize_time(args[i], bUnit);
				if (possible > lrChd) { lrChd = possible; biggest = i; }
			}
			else if (!existAnyof(args[i], ";:<>"))
				accmTime = (accmTime + atomize_time(args[i], bUnit)) % mbpC;

			makeChords(args, i, !activeChord);
			if (existElement(args[i], '>')) {
				std::string aux(args[i]);
				deleteChars(aux, ">");
				if (biggest == i) biggest++;
				replaceElements(args, i, { ">", aux });
				if (begin < biggest)
					switchElements(args, begin, biggest);
				accmTime = (accmTime + lrChd) % mbpC;
				activeChord = false;
			}
			if (existElement(args[i], ';')) {
				replaceElements(args, i, deconst2P(mbpC - accmTime));
				accmTime = 0;
			}
		}
	}

	BIT8* compileMelody(std::string str, char scale, size_t nBeats, BIT8 bUnit) {
		std::vector<std::string> vec_str = split(str);
		unpackChords(vec_str, nBeats, bUnit);
		std::vector<BIT8> vec; BIT8 currStatus = 0x40;
		std::cout << "Begining Compiling process" << std::endl;
		for (std::string arg : vec_str) {
			if (update_status(arg, currStatus))
				continue;
			vec.push_back(currStatus);
			vec.push_back(atomize_tone(arg, scale));
			vec.push_back(atomize_time(arg, bUnit));
		}
		vec.push_back(0x7F); vec.push_back(0); vec.push_back(0);
		return VtoA(vec);
	}
	BIT8* compileFile(std::string path) {
		std::ifstream file(path);
		if (!file.is_open())
			return NULL;
		std::string buffer = "", args = "";
		int pos = 0;
		while (std::getline(file, buffer)) {
			pos = buffer.find_first_not_of(" \t\r");
			if (pos != std::string::npos)
				buffer.erase(0, pos);
			pos = locateElement(buffer, "//", 0);
			if (pos != -1)
				buffer.erase(pos, buffer.size());

			if (!buffer.empty())//
				args.append(buffer + " ");
		}
		return compileMelody(args, C4, 4, NG);
	}
};
class Variable {
	private: std::string id, value;

	public: Variable(const std::string id, const std::string value) {
		this->id = id;
		this->value = value;
	}
	public: void setValue(const std::string value) {
		this->value = value;
	}
	public: std::string getString() {
		return this->value;
	}
	public: std::string getID() {
		return this->id;
	}
	public: int getInt() {
		try {
			return std::stoi(this->value);
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "Error| Value: " << this->value << " Cannot be cast as Integer" << std::endl;
		}
		return 0;
	}
	public: float getFloat() {
		try {
			return std::stof(this->value);
		}
		catch (const std::invalid_argument& e) {
			std::cerr << "Error| Value: " << this->value << " Cannot be cast as Float" << std::endl;
		}
		return 0.0f;
	}
	public: bool getBool() {
		return !this->value.compare("true");
	}
	public: void print(size_t tabs) {
		std::cout << std::string(tabs, '\t') << "Id: " << this->id << "\t Value: " << this->value << std::endl;
	}
};
class Instruction {
	private: char type;
	private: std::vector<Variable> values;

	public: Instruction(const char type) {
		this->type = type;
		switch(this->type){
			case 0:
				for (std::string p : ins_play) {
					Variable v(p, "");
					this->values.push_back(v);
				}
				break;
			case 1:
				for (std::string p : ins_pause) {
					Variable v(p, "");
					this->values.push_back(v);
				}
				break;
			case 2:
				for (std::string p : ins_stop) {
					Variable v(p, "");
					this->values.push_back(v);
				}
				break;
		}
	}

	public: void setValue(const std::string id, const std::string value) {
		for(Variable& v: this->values)
			if (!v.getID().compare(id)) {
				v.setValue(value);
				return;
			}
	}
	public: Variable getValue(const std::string id) {
		for (Variable v : this->values) {
			if (!v.getID().compare(id))
				return v;
		}
		return null_var;
	}
	public: bool identify(const std::string comp) {
		return !this->getValue("id").getString().compare(comp);
	}
	public: char getType() {
		return this->type;
	}
	public: void print(size_t tabs) {
		std::cout << std::string(tabs, '\t') << "Tipo de Instruccion: " << (int)type << std::endl << std::string(tabs, '\t') << "Values: " << std::endl;
		for (Variable v : this->values)
			v.print(tabs + 1);
	}
	};
class MelodyInterface {
	public: std::string id, scale, beats, compass, sequence, currSequence;
	public: std::vector<std::string> vars;

	public: MelodyInterface(const std::string propeties, const std::string sequence, const std::string arguments) {
		this->sequence = sequence;
		this->currSequence = sequence;
		std::sregex_iterator sr_var = std::sregex_iterator(propeties.begin(), propeties.end(), regex_prop);
		std::sregex_iterator sr_args = std::sregex_iterator(arguments.begin(), arguments.end(), regex_args);
		std::sregex_iterator end = std::sregex_iterator();
		std::smatch match;
		this->id = "NONE";
		this->scale = "0";
		this->beats = "4";
		this->compass = "4";
		for (std::sregex_iterator i = sr_var; i != end; ++i) {
			match = *i;
			this->setPropierty(match[1].str(), match[2].str());
		}
		for (std::sregex_iterator i = sr_args; i != end; ++i) {
			match = *i;
			this->vars.push_back(match[1].str());
		}
	}
	public: void setPropierty(const std::string prop, std::string value) {
		if (value.empty())
			return;
		if (!prop.compare("id"))
			this->id = value;
		if (!prop.compare("scale"))
			this->scale = value;
		if (!prop.compare("beats"))
			this->beats = value;
		if (!prop.compare("compass"))
			this->compass = value;
	}
	public: std::vector<std::string> getVarTags() {
		return this->vars;
	}
	public: void replaceVars(std::vector<Variable> variables) {
		size_t var_index = 0; this->currSequence.clear();
		for (size_t i = 0; i < this->sequence.size(); i++)
			if (this->sequence[i] == '$') {
				if (var_index < variables.size())
					this->currSequence += variables[var_index++].getString();
				else
					this->currSequence += "s";
			}
			else
				this->currSequence += this->sequence[i];

	}
	public: std::string getID() {
		return this->id;
	}
	public: void print(size_t tabs) {
		std::cout << std::string(tabs, '\t') << "Propieties: " << std::endl;
		std::cout << std::string(tabs, '\t') << "ID: " << id << std::endl;
		std::cout << std::string(tabs, '\t') << "Scale: " << scale << std::endl;
		std::cout << std::string(tabs, '\t') << "Beats: " << beats << std::endl;
		std::cout << std::string(tabs, '\t') << "Compass: " << compass << std::endl;
		std::cout << std::string(tabs, '\t') << "Sequence: " << sequence << std::endl;
		std::cout << std::string(tabs, '\t') << "Variables: ";
		for (std::string v : this->vars) std::cout << v << " ";
		std::cout << std::endl;
	}
};

const Variable null_var("null", "");
const MelodyInterface null_melody("<id='null'>", "", "");

class ExternalBlock {
	public: std::string id;
	public: std::vector<MelodyInterface> melodies;
	public: std::vector<Variable> vars;
	public: std::vector<Instruction> orders;

	public: ExternalBlock(const std::string id, const std::string args) {
		this->id = id;
		std::sregex_iterator sr_var = std::sregex_iterator(args.begin(), args.end(), regex_varDec);
		std::sregex_iterator sr_melody = std::sregex_iterator(args.begin(), args.end(), regex_melody);
		std::sregex_iterator evn = std::sregex_iterator(args.begin(), args.end(), regex_event);
		std::sregex_iterator end = std::sregex_iterator();
		std::smatch match;

		for (std::sregex_iterator i = sr_var; i != end; ++i) {
			match = *i;
			std::string content = match[1].str();
			std::sregex_iterator prop_begin = std::sregex_iterator(content.begin(), content.end(), regex_prop);
			for (std::sregex_iterator i = prop_begin; i != end; ++i) {
				std::smatch mp = *i;
				Variable var(mp[1].str(), mp[2].str());
				this->vars.push_back(var);
			}
		}
		for (std::sregex_iterator i = sr_melody; i != end; ++i) {
			match = *i;
			MelodyInterface mi(match[2].str(), match[3].str(), match[4].str());
			this->melodies.push_back(mi);
		}
		for (std::sregex_iterator i = evn; i != end; ++i) {

			match = *i; char type = 0;
			if (!match[1].str().compare("PLAY")) {
				type = 0;
			}
			if (!match[1].str().compare("PAUSE")) {
				type = 1;
			}
			if (!match[1].str().compare("STOP")) {
				type = 2;
			}
			std::string p = match[2].str();
			Instruction in(type);
			std::sregex_iterator arguments = std::sregex_iterator(p.begin(), p.end(), regex_prop);
			for (std::sregex_iterator j = arguments; j != end; ++j) {
				std::smatch prop = *j;
				in.setValue(prop[1].str(), prop[2].str());
			}
			this->orders.push_back(in);
		}
	}
	public: void print(size_t tabs) {
		std::cout << std::string(tabs, '\t') << "Id: " << this->id << std::endl;
		std::cout << std::string(tabs, '\t') << "Variables: " << std::endl;
		for (Variable v : this->vars) v.print(tabs + 1);
		std::cout << std::string(tabs, '\t') << "Melodies: " << std::endl;
		for (MelodyInterface m : this->melodies) m.print(tabs + 1);
		std::cout << std::string(tabs, '\t') << "Orders: " << this->orders.size() << std::endl;
		for (Instruction i : this->orders) i.print(tabs + 1);
	}

	public: Variable getVariable(const std::string id) {
		for (Variable v : this->vars)
			if (!v.getID().compare(id))
				return v;
		return null_var;
	}
	public: MelodyInterface getMelody(const std::string id) {
		for (MelodyInterface m : this->melodies)
			if (!m.getID().compare(id))
				return m;
		return null_melody;
	}
};

const ExternalBlock null_block("null", "");

class FileReader {
	public: std::string id;
	public: std::vector<std::string> paths;
	public: std::vector<Variable> variables, instruments;
	public: std::vector<MelodyInterface> melodies;
	public: std::vector<ExternalBlock> groups, scripts;

	public: FileReader() {
		this->resetData();
	}
	public: void detectExternalBlocks(std::string str) {
		std::sregex_iterator grps = std::sregex_iterator(str.begin(), str.end(), regex_group);
		std::sregex_iterator crpt = std::sregex_iterator(str.begin(), str.end(), regex_script);
		std::sregex_iterator end = std::sregex_iterator();
		std::smatch match;

		for (std::sregex_iterator i = grps; i != end; ++i) {
			match = *i;
			ExternalBlock eb(match[1].str(), match[2].str());
			groups.push_back(eb);
		}
		str = std::regex_replace(str, regex_group, "");
		for (std::sregex_iterator i = crpt; i != end; ++i) {
			match = *i;
			ExternalBlock eb(match[1].str(), match[2].str());
			scripts.push_back(eb);
		}
		str = std::regex_replace(str, regex_script, "");
		if (std::regex_search(str, match, regex_mscript)) {
			ExternalBlock ms("__init__", match[1].str());
			scripts.push_back(ms);
		}
		str = std::regex_replace(str, regex_mscript, "");

		std::sregex_iterator imports = std::sregex_iterator(str.begin(), str.end(), regex_import);
		std::sregex_iterator ins = std::sregex_iterator(str.begin(), str.end(), regex_inst);
		std::sregex_iterator mels = std::sregex_iterator(str.begin(), str.end(), regex_melody);
		std::sregex_iterator vars = std::sregex_iterator(str.begin(), str.end(), regex_varDec);

		for (std::sregex_iterator i = imports; i != end; ++i) {
			if (std::regex_search(str, match, regex_path))
				paths.push_back(match[1].str());
		}
		for (std::sregex_iterator i = ins; i != end; ++i) {
			match = *i;
			std::string content = match[1].str();
			std::sregex_iterator prop_begin = std::sregex_iterator(content.begin(), content.end(), regex_prop);
			for (std::sregex_iterator i = prop_begin; i != end; ++i) {
				std::smatch mp = *i;
				Variable ii(mp[1].str(), mp[2].str());
				instruments.push_back(ii);
			}
		}
		for (std::sregex_iterator i = vars; i != end; ++i) {
			match = *i;
			std::string content = match[1].str();
			std::sregex_iterator prop_begin = std::sregex_iterator(content.begin(), content.end(), regex_prop);
			for (std::sregex_iterator i = prop_begin; i != end; ++i) {
				std::smatch mp = *i;
				Variable var(mp[1].str(), mp[2].str());
				variables.push_back(var);
			}
		}
		for (std::sregex_iterator i = mels; i != end; ++i) {
			match = *i;
			MelodyInterface mi(match[2].str(), match[3].str(), match[4].str());
			this->melodies.push_back(mi);
		}
	}
	public: void compileFile(const std::string archive_path) {
		std::ifstream archive(archive_path);
		if (!archive.is_open()) {
			std::cerr << "Error al abrir el archivo: " << archive_path << std::endl;
			return;
		}

		int position = (int)archive_path.find_last_of("\\/");
		if (position != std::string::npos)
			this->id = archive_path.substr(position + 1, archive_path.find_last_of('.') - position - 1);
		else
			this->id = archive_path.substr(0, archive_path.find_last_of('.') - 1);

		std::stringstream buffer;
		buffer << archive.rdbuf();
		archive.close();

		std::string text = buffer.str();
		clearString(text);
		detectExternalBlocks(text);
	}

	public: void resetData() {
		this->paths.clear();
		this->variables.clear();
		this->instruments.clear();
		this->groups.clear();
		this->scripts.clear();
	}
	public: void print() {
		std::cout << "FileName: " << this->id << std::endl << "***Imports***" << std::endl;
		for (std::string s : paths)
			std::cout << s << std::endl;
		std::cout << "***Instruments***" << std::endl;
		for (Variable i : instruments)
			i.print(1);
		std::cout << std::endl << "***Global Melodies***" << std::endl;
		for (MelodyInterface m : melodies)
			m.print(1);
		std::cout << std::endl << "***Global Variables***" << std::endl;
		for (Variable v : variables)
			v.print(1);
		std::cout << std::endl << "***Groups***" << std::endl;
		for (ExternalBlock g : groups)
			g.print(1);
		std::cout << std::endl << "***Scripts***" << std::endl;
		for (ExternalBlock s : scripts)
			s.print(1);
	}

	public: Variable getVariable(const std::string id, const std::string relative) {
		if (!relative.empty()) {
			for (ExternalBlock e : this->groups)
				if (!e.id.compare(relative))
					return e.getVariable(id);
			return null_var;
		}
		switch (countElement(id, ':')) {
		case 0:
			for (Variable v : this->variables)
				if (!v.getID().compare(id))
					return v;
			return null_var;
		case 1: {
			std::string idBlock = id.substr(0, id.find(':'));
			std::string idVar = id.substr(id.find(':') + 1);
			for (ExternalBlock e : this->groups)
				if (!e.id.compare(idBlock))
					return e.getVariable(idVar);
			return null_var;
		}
		default:
			return null_var;
		}
	}
	public: MelodyInterface getMelody(const std::string id, const std::string relative) {
		if (!relative.empty()) {
			for (ExternalBlock e : this->groups)
				if (!e.id.compare(relative))
					return e.getMelody(id);
			return null_melody;
		}
		switch (countElement(id, ':')) {
		case 0:
			for (MelodyInterface m : this->melodies)
				if (!m.getID().compare(id))
					return m;
			return null_melody;
		case 1: {
			std::string idBlock = id.substr(0, id.find(':'));
			std::string idVar = id.substr(id.find(':') + 1);
			for (ExternalBlock e : this->groups)
				if (!e.id.compare(idBlock))
					return e.getMelody(idVar);
			return null_melody;
		}
		default:
			return null_melody;
		}
	}
	public: ExternalBlock getExternalBlock(const std::string id){
		for (ExternalBlock ex : this->scripts)
			if (!ex.id.compare(id))
				return ex;
		return null_block;
	}
};

class CompileMaster {
	public: std::vector<FileReader> files;

	public: void compileFile(const std::string init) {
		std::stack<std::string> to_compile;
		to_compile.push(init);
		do {
			FileReader fr;
			fr.compileFile(to_compile.top());
			to_compile.pop();
			for (std::string s : fr.paths)
				to_compile.push(s);
			this->files.push_back(fr);
		} while (!to_compile.empty());

	}
	public: void print() {
		for (FileReader fr : this->files)
			fr.print();
	}
	public: void updateMelody(MelodyInterface& mi){
		std::vector<Variable> vars_of_mel;
		for (std::string vt : mi.getVarTags()) {
			Variable v = this->getVariable(vt, "");
			if (v.getID().compare("null"))
				vars_of_mel.push_back(v);
		}
		mi.replaceVars(vars_of_mel);
	}
	public: BIT8* compileMelody(const std::string id_mel) {
		MelodyInterface melody = this->getMelody(id_mel, "");
		updateMelody(melody);
		std::cout << "Compiled Melody: " << melody.currSequence << std::endl;
		return MelodyComp::compileMelody(melody.currSequence, MelodyComp::getValue(melody.scale), MelodyComp::getValue(melody.beats), MelodyComp::getValue(melody.compass));
	}
	public: Variable getVariable(const std::string id, const std::string relative) {
		if (!relative.empty()) {
			for (ExternalBlock e : this->files[0].groups)
				if (!e.id.compare(relative))
					return e.getVariable(id);
			return null_var;
		}
		switch (countElement(id, ':')) {
			case 0:
				for (Variable v : this->files[0].variables)
					if (!v.getID().compare(id))
						return v;
				return null_var;
			case 1: {
				std::string idBlock = id.substr(0, id.find_first_of(':'));
				std::string idVar = id.substr(id.find_first_of(':') + 1);
				for (ExternalBlock e : this->files[0].groups)
					if (!e.id.compare(idBlock))
						return e.getVariable(idVar);

				for (FileReader fr : this->files)
					if (!fr.id.compare(idBlock))
						return fr.getVariable(idBlock, "");

				return null_var;
			}
			case 2: {
				std::string idBlock = id.substr(0, id.find_first_of(':'));
				std::string idVar = id.substr(id.find_first_of(':') + 1);
				for (FileReader fr : this->files)
					if (!fr.id.compare(idBlock))
						return fr.getVariable(idBlock, "");

				return null_var;
			}
			default:
				return null_var;
		}
	}
	public: MelodyInterface getMelody(const std::string id, const std::string relative) {
		if (!relative.empty()) {
			for (ExternalBlock e : this->files[0].groups)
				if (!e.id.compare(relative))
					return e.getMelody(id);
			return null_melody;
		}
		switch (countElement(id, ':')) {
			case 0:
				for (MelodyInterface m : this->files[0].melodies)
					if (!m.getID().compare(id))
						return m;
				return null_melody;
			case 1: {
				std::string idBlock = id.substr(0, id.find_first_of(':'));
				std::string idVar = id.substr(id.find_first_of(':') + 1);
				for (ExternalBlock e : this->files[0].groups)
					if (!e.id.compare(idBlock))
						return e.getMelody(idVar);

				for (FileReader fr : this->files)
					if (!fr.id.compare(idBlock))
						return fr.getMelody(idBlock, "");

				return null_melody;
			}
			case 2: {
				std::string idBlock = id.substr(0, id.find_first_of(':'));
				std::string idVar = id.substr(id.find_first_of(':') + 1);
				for (FileReader fr : this->files)
					if (!fr.id.compare(idBlock))
						return fr.getMelody(idBlock, "");
			}
		}
		return null_melody;
	}
	public: ExternalBlock getBlock(const std::string id) {
		switch (countElement(id, ':')) {
			case 0:
				for (ExternalBlock b : this->files[0].scripts)
					if (!b.id.compare(id))
						return b;
				return null_block;
			case 1: {
				std::string idBlock = id.substr(0, id.find_first_of(':'));
				std::string idVar = id.substr(id.find_first_of(':') + 1);
				for (FileReader f : this->files)
					if (!f.id.compare(idBlock))
						return f.getExternalBlock(idVar);
				return null_block;
			}
		}
	}
};