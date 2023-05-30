#include <iostream>
#include "CostumeConsole.h"

int main() {
	AdvCon::AdvanceConsole* AC = AdvCon::AdvanceConsole::getAC();
	std::cout << (AC != nullptr) << std::endl;
	std::cout << "Hola Mundo\n";
	AC->clearConsole();
	std::cout << "Nuevo Mensaje\n";
	//std::cout << " Paradigma con una longitud superior a 20\n";
	return 0;
}