#pragma once
#include<iostream>
#include<string>
#include<fstream>
#include<vector>

#ifndef BIT8
#define BIT8 unsigned char
#endif

class MelodyComp {
	private: static std::string HEX;
	private: BIT8 currStatus = 0x40;

	public: static const BIT8 C0 = 0x00, C1 = 0x0C, C2 = 0x18, C3 = 0x24, C4 = 0x30, C5 = 0x3C;
	public: static const BIT8 RN = 0x80, BL = 0x40, NG = 0x20, CH = 0x10, SH = 0x08, QH = 0x04;

	public: static BIT8* vtoa(std::vector<BIT8> vec) {
		if (vec.size() == 0)
			return nullptr;
		BIT8* arr = new BIT8[vec.size()];
		for (size_t i = 0; i < vec.size(); i++)
			arr[i] = vec[i];
		return arr;
	}
	public: static void printArr(BIT8* arr, size_t size) {
		for (size_t i = 0; i < size; i++)
			std::cout << "0x" << HEX[arr[i] / 16] << HEX[arr[i] % 16] << ((i + 1) % 3 ? ", " : "\n");
	}

	public: static void toUpperCase(std::string& str) {
		for (size_t i = 0; i < str.size(); i++)
			if (str[i] > 0x60 && str[i] < 0x7A)
				str[i] &= 0xDF;

	}
	public: static std::vector<std::string> split(std::string args, std::string split = " \t\n", std::string splitCap = ",;:") {
		std::vector<std::string> splits; std::string splitter = split + splitCap;
		size_t pos = args.find_first_of(splitter), begin = 0;
		while (pos != std::string::npos) {
			if (pos - (int)begin)//Si existe un argumento
				splits.push_back(args.substr(begin, pos - (int)begin));
			if (countElement(",;:", args[pos]))//Si es un split simbolico
				splits.push_back("" + args[pos]);
			begin = pos + 1;
			pos = args.find_first_of(splitter, begin);
		}
		splits.push_back(args.substr(begin, args.size() - (int)begin));
		return splits;
	}
	public: static std::string delete_chars(std::string strn, std::string target) {
		std::string str(strn);
		size_t pos = str.find_first_of(target);
		while (pos != std::string::npos) {
			str.erase(pos, 1);
			pos = str.find_first_of(target);
		}
		return str;
	}

	public: static int countElement(std::string arg, char target) {
		int count = 0;
		size_t pos = arg.find_first_of(target);
		while (pos != std::string::npos) {
			count++;
			pos = arg.find_first_of(target, pos + 1);
		}
		return count;
	}
	public: static int locateElement(std::string str, const std::string arg) {
		size_t size_arg = arg.size(), i = str.find_first_of(arg[0]);
		while (i != std::string::npos && (i + size_arg) <= str.size()) {
			if (!str.substr(i, size_arg).compare(arg))
				return i;
			i = str.find_first_of(arg[0], i + 1);
		}
		return -1;
	}

	public: static size_t near2P(const size_t num) {
		size_t n = 0;
		while (num > 1 << n) n++;
		return n;
	}
	public: static std::vector<BIT8> deconst_2P(const size_t num) {
		size_t z = num % 128, m = num / 128, n = near2P(num) + 1;
		std::vector<BIT8> vec;
		do {
			n--;
			if (z >= 1 << n) {
				vec.push_back(1 << n);
				z -= 1 << n;
			}
		} while (n);
		while (m) {
			vec.push_back(128);
			m--;
		}
		return vec;
	}
	/*Significado de los Bits de Status
	7 := si es 1, El generador de notas debe esperar a otras notas  hasta recibir un estatus de 0 :: 0xF0
	6 := si es 0, Esta nota corresponde a un silencio ignora el tono y solo aumenta la cuenta del tiempo :: 0x80
	5 := si es 1, Indica que esta última nota sobrepasó los ticks asignados a la frase ó es la última de esta y debe sonar distintiva
	4 := si es 1, Indica que esta nota tiene una duración de 1.5 veces su valor asignado, si bien esta será reflejada en el BIT8 de tiempo, se debe indicar para el instrumento
	[3 - 0] := estos bits corresponden a los estados que puede adoptar cierta nota en determinado instrumento*/
	private: void update_status(std::string arg) {
		if (locateElement(arg, "SILEN") != -1 || countElement(arg,','))
			currStatus &= 0xBF;
		else
			currStatus |= 0x40;
		if (countElement(arg, '<'))
			currStatus |= 0x80;
		if (countElement(arg, '>'))
			currStatus &= 0x7F;

	}
	private: BIT8 atomize_tone(std::string arg, BIT8 scale) {
		int adj = 12 * countElement(arg, '+') + countElement(arg, '#') - (countElement(arg, 'B') + 12 * countElement(arg, '-'));
		if (locateElement(arg, "LA") != -1)
			return 0x00 + scale + adj; //
		if (locateElement(arg, "SI") != -1)
			return 0x02 + scale + adj;
		if (locateElement(arg, "DO") != -1)
			return 0x03 + scale + adj;
		if (locateElement(arg, "RE") != -1)
			return 0x05 + scale + adj;
		if (locateElement(arg, "MI") != -1)
			return 0x07 + scale + adj;
		if (locateElement(arg, "FA") != -1)
			return 0x08 + scale + adj;
		if (locateElement(arg, "SOL") != -1)
			return 0x0A + scale + adj;
		return 0x00;
	}
	private: BIT8 atomize_time(std::string arg, BIT8 bUnit) {
		size_t pos = arg.find_first_of("_.");
		if (pos == std::string::npos)
			return bUnit;
		if ((pos + 1) == arg.size())
			return bUnit + (arg[pos] == '.' ? bUnit >> 1 : 0x00);
		size_t t = std::stoi(arg.substr(pos + 1, arg.size() - (int)pos - 1));
		if (t < 2) return 0x80 + (arg[pos] == '.' ? 0x40 : 0x00);
		if (t < 4) return 0x40 + (arg[pos] == '.' ? 0x20 : 0x00);
		if (t < 8) return 0x20 + (arg[pos] == '.' ? 0x10 : 0x00);
		if (t < 16) return 0x10 + (arg[pos] == '.' ? 0x08 : 0x00);
		if (t < 32) return 0x08 + (arg[pos] == '.' ? 0x04 : 0x00);
		if (t < 64) return 0x04 + (arg[pos] == '.' ? 0x02 : 0x00);
		return 0x00;
	}
	private: std::string stract_note(std::string arg) {
		size_t pos = arg.find_first_of("_.");
		if (pos == std::string::npos)
			return arg;
		return arg.substr(0, pos - 1);
	}
	private: void unpackSimbols(std::vector<std::string>& args, size_t nBeats, BIT8 bUnit) {
		size_t begin = 0, biggest = 0, max_b = nBeats * bUnit, curr_b = 0;
		BIT8 max = 0x01, curr = 0x00; bool active = false; std::string last_note = "silen";
		for (size_t i = 0; i < args.size(); i++) {
			curr = atomize_time(args[i], bUnit);
			//Reordenamiento de Acordes Personalizados
			if (countElement(args[i], '<')) { //Se inicia un Acorde
				begin = i; biggest = i;
				max = 0x01; active = true;
				curr_b = (curr_b + curr) % max_b;
			}
				
			if (active) {
				if (curr > max) { //Si hay un Acorde activo compara el Valor Más alto
					max = curr;
					biggest = i;
				}
			}
			else { curr_b = (curr_b + curr) % max_b; last_note = args[i]; }

			if (countElement(args[i], '>')) {//Se cierra el Acorde
				if (begin < biggest) {//Si la nota más duradera, no es el inicio del acorde
					std::string big_one = delete_chars(args[biggest], ">");
					std::string first = delete_chars(args[begin], "<") + (i == biggest ? ">" : "");
					//Se realiza un intercambio de lugares		   <b...B...>  | <b...B>
					args.erase(args.begin() + begin);			//   ...B...>  |   ...B>
					args.erase(args.begin() + biggest - 1);		//   ...>      |   ...
					args.insert(args.begin() + biggest - 2, first);//...b...>  |   ...b>
					args.insert(args.begin() + begin, big_one); // <B...b...>  | <B...b>
				}
				active = false;
			}
			//Localización de símbolos de Relleno
			if (!active && countElement(args[i], ';')) { //Completar el Segmento con Silencio
				args.erase(args.begin() + i);
				for (BIT8 t : deconst_2P(max_b - (int)curr_b)) {
					args.insert(args.begin() + i, "silen_" + (t / 128));
					i++;
				}
			}
			if (!active && countElement(args[i], ':')) { //Completar el Segmento con la Nota Anterior
				args.erase(args.begin() + i);
				last_note = stract_note(last_note);
				for (BIT8 t : deconst_2P(max_b - (int)curr_b)) {
					args.insert(args.begin() + i, last_note + "_" + std::string("" + (t / 128)));
					i++;
				}
			}
		}
	}

	public: BIT8* compileFile(std::string path) {
		std::ifstream file(path);
		if (!file.is_open())
			return NULL;
		std::string buffer="", args = "";
		int pos = 0;
		while (std::getline(file, buffer)) {
			//std::cout << buffer << std::endl;
			pos = buffer.find_first_not_of(" \t\r");//Eliminar Espacios, tabulaciones, etc.
			if(pos != std::string::npos)
				buffer.erase(0, pos);
			pos = locateElement(buffer, "//");//Localizar comentarios en linea
			if (pos != -1)
				buffer.erase(pos, buffer.size());
			//std::cout << buffer << std::endl;

			if (!buffer.empty())
				args.append(buffer+" ");
		}
		//Construir un metodo para leer directivas de compilación//
		return vitalComp(args);
	}
	public: BIT8* vitalComp(std::string str, size_t& size, BIT8 scale = C4, size_t nBeats = 4, BIT8 bUnit = NG) {
		std::vector<BIT8> vec;
		toUpperCase(str);
		std::vector<std::string> args = split(str);
		unpackSimbols(args, nBeats, bUnit);
		for (std::string arg : args) {
			update_status(arg);
			vec.push_back(currStatus);
			vec.push_back(atomize_tone(arg, scale));
			vec.push_back(atomize_time(arg, bUnit));
		}
		vec.push_back(0x7F);
		size = vec.size();
		return vtoa(vec);
	}
	public: BIT8* vitalComp(std::string str, BIT8 scale = C4, size_t nBeats = 4, BIT8 bUnit = NG) {
		std::vector<BIT8> vec;
		toUpperCase(str);
		std::vector<std::string> args = split(str);
		unpackSimbols(args, nBeats, bUnit);
		for (std::string arg : args) {
			update_status(arg);
			vec.push_back(currStatus);
			vec.push_back(atomize_tone(arg, scale));
			vec.push_back(atomize_time(arg, bUnit));
		}
		vec.push_back(0x7F);
		return vtoa(vec);
	}
};

std::string MelodyComp::HEX = "0123456789ABCDEF";