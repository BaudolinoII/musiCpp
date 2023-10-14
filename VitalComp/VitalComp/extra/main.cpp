#include <iostream>
#include <string>
#include <Windows.h>
#include <cstdio>

#include "CostumeConsole.h"

AdvCon::AdvanceConsole* AC = AdvCon::AdvanceConsole::getAC();
//
//const size_t scrWidth = 120;
//const size_t scrHeight = 40;

int main() {
	AC->clearConsole();
	//AC->drawMainFrame();
	AC->setTextAttributes(2);
	std::cout << "Hola Mundo\n";
	return 0;
}
