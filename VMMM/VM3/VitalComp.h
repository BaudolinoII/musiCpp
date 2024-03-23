#pragma once
#include<iostream>
#include<string>
#include<vector>

#ifndef BIT8
#define BIT8 unsigned char
#endif

/*const BIT8 MI = 0x0D, FA = 0x0E, FA_M = 0x0F, SOL = 0x10, SOL_M = 0x11, LA = 0x06;
const BIT8 LA_M = 0x07, SI = 0x08, DO = 0x09, DO_M = 0x0A, RE = 0x0B, RE_M = 0x0C;
const BIT8 RD = 0x80, BL = 0x40, NG = 0x20, CH = 0x10, SH = 0x08;
const BIT8 SINGLE = 0x40, CHORD = 0xC0, SILEN = 0x00, FIN = 0x7F;*/

class MelodyComp {
private: static std::string HEX;
private: BIT8 currStatus = 0x40;

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
public: static std::vector<std::string> split(std::string args, std::string split = " ") {
	std::vector<std::string> splits;
	size_t pos = args.find_first_of(split), begin = 0;
	while (pos != std::string::npos) {
		if (pos - (int)begin)
			splits.push_back(args.substr(begin, pos - (int)begin));
		begin = pos + 1;
		pos = args.find_first_of(split, begin);
	}
	splits.push_back(args.substr(begin, args.size() - (int)begin));
	return splits;
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
public: static bool existElement(std::string str, const std::string arg) {
	size_t size_arg = arg.size(), i = str.find_first_of(arg[0]);

	while (i != std::string::npos && (i + size_arg) <= str.size()) {
		if (!str.substr(i, size_arg).compare(arg))
			return true;
		i = str.find_first_of(arg[0], i + 1);
	}
	return false;
}

	  /*Significado de los Bits de Status
		7 := si es 1, El generador de notas debe esperar a otras notas  hasta recibir un estatus de 0 :: 0xF0
		6 := si es 0, Esta nota corresponde a un silencio ignora el tono y solo aumenta la cuenta del tiempo :: 0x80
		5 := si es 1, Indica que esta última nota sobrepasó los ticks asignados a la frase ó es la última de esta y debe sonar distintiva
		4 := si es 1, Indica que esta nota tiene una duración de 1.5 veces su valor asignado, si bien esta será reflejada en el BIT8 de tiempo, se debe indicar para el instrumento
		[3 - 0] := estos bits corresponden a los estados que puede adoptar cierta nota en determinado instrumento*/
private: void update_status(std::string arg) {
	if (existElement(arg, "SILEN"))
		currStatus &= 0xBF;
	else
		currStatus |= 0x40;
	if (countElement(arg, '<'))
		currStatus |= 0x80;
	if (countElement(arg, '>'))
		currStatus &= 0x7F;

}
private: BIT8 atomize_tone(std::string arg, BIT8 scale = 0x06) {
	int adj = countElement(arg, '#') - countElement(arg, 'B');
	if (existElement(arg, "LA"))
		return 0x00 + scale + adj;
	if (existElement(arg, "SI"))
		return 0x02 + scale + adj;
	if (existElement(arg, "DO"))
		return 0x03 + scale + adj;
	if (existElement(arg, "RE"))
		return 0x05 + scale + adj;
	if (existElement(arg, "MI"))
		return 0x07 + scale + adj;
	if (existElement(arg, "FA"))
		return 0x08 + scale + adj;
	if (existElement(arg, "SOL"))
		return 0x0A + scale + adj;
	return 0x00;
}
private: BIT8 atomize_time(std::string arg, BIT8 def = 0x20) {
	size_t pos = arg.find_first_of("_.");
	if (pos == std::string::npos)
		return def;
	if ((pos + 1) == arg.size())
		return def + (arg[pos] == '.' ? def >> 1 : 0x00);
	size_t t = std::stoi(arg.substr(pos + 1, arg.size() - (int)pos - 1));
	if (t < 2) return 0x80 + (arg[pos] == '.' ? 0x40 : 0x00);
	if (t < 4) return 0x40 + (arg[pos] == '.' ? 0x20 : 0x00);
	if (t < 8) return 0x20 + (arg[pos] == '.' ? 0x10 : 0x00);
	if (t < 16) return 0x10 + (arg[pos] == '.' ? 0x08 : 0x00);
	if (t < 32) return 0x08 + (arg[pos] == '.' ? 0x04 : 0x00);
	if (t < 64) return 0x04 + (arg[pos] == '.' ? 0x02 : 0x00);
	return 0x00;
}
private: void sortChords(std::vector<std::string>& args, BIT8 def = 0x20) {
	size_t begin = 0, biggest = 0;
	BIT8 max = 0x01; bool active = false;
	for (size_t i = 0; i < args.size(); i++) {
		if (countElement(args[i], '<')) {
			begin = i; biggest = i;
			max = 0x01; active = true;
		}
		BIT8 curr = atomize_time(args[i], def);
		if (active && curr > max) {
			max = curr;
			biggest = i;
		}
		if (countElement(args[i], '>')) {
			std::string temp = args[begin].substr(1) + (i == biggest ? ">" : "");
			std::string m = "<" + (i == biggest ? args[biggest].substr(0, args[biggest].size() - 1) : args[biggest]);
			args.erase(args.begin() + begin);
			args.erase(args.begin() + biggest - 1);
			args.insert(args.begin() + begin, m);
			args.insert(args.begin() + biggest, temp);
			for (std::string s : args)
				std::cout << s << " ";
			std::cout << std::endl;
			active = false;
		}
	}
}

public: BIT8* vitalComp(std::string str, size_t& size) {
	std::vector<BIT8> vec;
	toUpperCase(str);
	std::vector<std::string> args = split(str);
	sortChords(args);
	for (std::string arg : args) {
		update_status(arg);
		vec.push_back(currStatus);
		vec.push_back(atomize_tone(arg));
		vec.push_back(atomize_time(arg));
	}
	vec.push_back(0x7F);
	size = vec.size();
	return vtoa(vec);
}
};

std::string MelodyComp::HEX = "0123456789ABCDEF";