#include<iostream>
#include<string>
#include<fstream>
#include<vector>

#include"VitalComp.hpp"

const std::vector<std::string> occ_scale_cat = { "La", "La#", "Si", "Do", "Do#", "Re", "Re#", "Mi", "Fa", "Fa#", "Sol", "Sol#" };
const std::vector<std::string> chdCms = { "13M","13","11M","11","9M","9","#9","B9","7M","7","/6","6","MAY","MAJ","MIN","MEN","AUM","AUG","DIS","DIM","SUS","5","#5","B5","4","3","2" };
const std::string HEX = "0123456789ABCDEF";
const std::string splitMark = " \t\n\r";
const std::string splitCap = ",;:";
const BIT8 C0 = 0x00, C1 = 0x0C, C2 = 0x18, C3 = 0x24, C4 = 0x30, C5 = 0x3C;
const BIT8 RN = 0x80, BL = 0x40, NG = 0x20, CH = 0x10, SH = 0x08, QH = 0x04, OH = 0x02, DH = 0x01;

bool MelodyComp::existElement(const std::string str, const std::string arg) {
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
bool MelodyComp::existAnyof(const std::string str, const std::string args) {
	return str.find(args) != std::string::npos;
}
bool MelodyComp::existAllof(const std::string str, const std::string args) {
	for (size_t i = 0; i < args.size(); i++)
		if (str.find(args[i]) == std::string::npos)
			return false;
	return true;
}
bool MelodyComp::existElement(const std::string str, const char arg) {
	return str.find_first_of(arg) != std::string::npos;
}

void MelodyComp::printArr(BIT8* arr) {
	for (size_t i = 0; arr[i] != 0x7F; i++)
		std::cout << "0x" << HEX[arr[i] >> 4] << HEX[arr[i] & 15] << ((i + 1) % 3 ? ", " : "\n");
}
void MelodyComp::toUpperCase(std::string& str) {
	for (size_t i = 0; i < str.size(); i++)
		if (str[i] > 0x60 && str[i] < 0x7A)
			str[i] &= 0xDF;
}
void MelodyComp::deleteChars(std::string& str, const std::string target) {
	size_t pos = str.find_first_of(target);
	while (pos != std::string::npos) {
		str.erase(pos, 1);
		pos = str.find_first_of(target);
	}
}
void MelodyComp::deleteElement(std::string& str, const std::string target) {
	if (str.empty())
		return;
	int loc = locateElement(str, target, 0);
	if (loc == -1)
		return;
	str.erase(loc, target.size());
}
void MelodyComp::replaceElements(std::vector<std::string>& vec_str, size_t& index, const std::vector<std::string> vec_tar) {
	vec_str.erase(vec_str.begin() + index);
	for (std::string tar : vec_tar)
		vec_str.insert(vec_str.begin() + index++, tar);
	index--;
}
void MelodyComp::switchElements(std::vector<std::string>& vec_str, size_t a, size_t b) {
	std::string aux(vec_str[a]), bux(vec_str[b]);
	vec_str.erase(vec_str.begin() + a);
	vec_str.insert(vec_str.begin() + a, bux);
	vec_str.erase(vec_str.begin() + b);
	vec_str.insert(vec_str.begin() + b, aux);
}

int MelodyComp::countElement(const std::string str, const char target) {
	int count = 0;
	size_t pos = str.find_first_of(target);
	while (pos != std::string::npos) {
		count++;
		pos = str.find_first_of(target, pos + 1);
	}
	return count;
}
int MelodyComp::locateElement(const std::string str, const std::string args, const size_t cap) {
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

size_t MelodyComp::near2P(const size_t num) {
	size_t n = 0;
	while (num > (1 << n)) n++;
	return n;
}

std::string MelodyComp::getUpperCase(const std::string str) {
	std::string upp = "";
	for (size_t i = 0; i < str.size(); i++)
		if (str[i] > 0x60 && str[i] < 0x7A)
			upp.append(1, str[i] & 0xDF);
		else
			upp.append(1, str[i]);

	return upp;
}
BIT8* MelodyComp::VtoA(std::vector<BIT8> vec) {
	if (vec.size() == 0)
		return nullptr;
	BIT8* arr = new BIT8[vec.size()];
	for (size_t i = 0; i < vec.size(); i++)
		arr[i] = vec[i];
	return arr;
}
std::vector<std::string> MelodyComp::split(const std::string args) {
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
std::vector<std::string> MelodyComp::deconst2P(const size_t num) {
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

/*Significado de los Bits de Status
7 := si es 1, Espera a colocar las notas hasta recibir un estatus de 0 :: 0xF0 []
6 := si es 1, El generador de notas debe esperar a otras notas  hasta recibir un estatus de 0 :: 0x80 ()
[3 - 0] := estos bits corresponden a los estados que puede adoptar cierta nota en determinado instrumento*/
std::string MelodyComp::extractChordTime(std::string& closeMark, BIT8 defaultTime) {
	size_t pos = closeMark.find('>');
	std::string aux(closeMark.substr(pos + 1));
	closeMark.erase(pos + 1);
	if (aux.empty())
		return "_" + std::to_string((int)(128 / defaultTime));
	return aux;
}

bool MelodyComp::update_status(std::string arg, BIT8& status) {
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
BIT8 MelodyComp::atomize_tone(std::string arg, BIT8 scale) {
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
	size_t pos = arg.find_first_of("SCDEFGHAB");
	if (pos == std::string::npos)
		return 0;
	switch (arg[pos]) {
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
BIT8 MelodyComp::atomize_time(std::string arg, BIT8 bUnit) {
	size_t pos = arg.find_first_of("_.");
	if (pos == std::string::npos)
		return bUnit;
	if ((pos + 1) == arg.size())
		return bUnit + (arg[pos] == '.' ? bUnit >> 1 : 0x00);
	size_t t = std::stoi(arg.substr(pos + 1, arg.size() - (int)pos - 1));
	//std::cout << "Tiempo a analizar " << t << std::endl;
	if (t < 2) return RN + (arg[pos] == '.' ? BL : 0x00);
	if (t < 4) return BL + (arg[pos] == '.' ? NG : 0x00);
	if (t < 8) return NG + (arg[pos] == '.' ? CH : 0x00);
	if (t < 16) return CH + (arg[pos] == '.' ? SH : 0x00);
	if (t < 32) return SH + (arg[pos] == '.' ? QH : 0x00);
	if (t < 64) return QH + (arg[pos] == '.' ? OH : 0x00);
	if (t < 128) return OH + (arg[pos] == '.' ? DH : 0x00);
	return DH;
}
void MelodyComp::makeChords(std::vector<std::string>& args, size_t& i, bool isNotOnChord = true) {
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
		chr.append(occ_scale_cat[chords[j] % 12]);
		if (!localTime.empty()) chr.append(localTime);
		commChord.push_back(chr);
	}
	if (isNotOnChord) {
		commChord.insert(commChord.begin(), "<");
		commChord.insert(commChord.end() - 1, ">");
	}
	replaceElements(args, i, commChord);
}
void MelodyComp::unpackChords(std::vector<std::string>& args, size_t nBeats, BIT8 scale, BIT8 bUnit) {
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
		else if (!existAnyof(args[i], ";:<>")) {
			accmTime = (accmTime + atomize_time(args[i], bUnit)) % mbpC;
			//std::cout << "Nodo: "<< args[i] << "\tACCM " << accmTime << "/" << mbpC << std::endl;
		}
		makeChords(args, i, !activeChord);
		if (existElement(args[i], '>')) {
			std::string aux(args[i]);
			deleteChars(aux, ">");
			if (biggest == i) biggest++;
			replaceElements(args, i, { ">", aux });
			if (begin < biggest)
				switchElements(args, begin, biggest);
			//std::cout << "Tiempo mas alto " << args[i] << " extraido: " << (int)atomize_time(args[i], lrChd) << " vs " << (int)lrChd << std::endl;
			accmTime = (accmTime + lrChd) % mbpC;
			//std::cout << "ACCM = " << accmTime << "/" << mbpC << std::endl;
			activeChord = false;
		}
		if (existElement(args[i], ';')) {
			//std::cout << "Espacio en rellenar " << (mbpC - accmTime) << std::endl;
			replaceElements(args, i, deconst2P(mbpC - accmTime));
			accmTime = 0;
		}
	}
}

BIT8* MelodyComp::compileMelody(std::string str, BIT8 scale, size_t nBeats, BIT8 bUnit) {
	std::vector<std::string> vec_str = split(str);
	unpackChords(vec_str, nBeats, scale, bUnit);
	std::vector<BIT8> vec; BIT8 currStatus = 0x40;
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
BIT8* MelodyComp::compileFile(std::string path) {
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
	//Construir un metodo para leer directivas de compilación//
	return compileMelody(args, MelodyComp::C4, 4, MelodyComp::NG);
}


