#include <iostream>
#include "Virtual_Instrument.h"
#include "NoiseMachine.h"
#include "VitalComp.h"

#ifndef BIT8
#define BIT8 unsigned char
#endif

int main() {
	Instrument_xml main_inst;
	if (0 > main_inst.load_document("./xml_samples/Experimental2.xml")) {
		std::cout << "No se encontro el archivo" << std::endl;
		return -1;
	}
	std::cout << "Instrumentos Cargados\n";

	/*VMMM::ConsoleSynth cs;
	cs.KeyBoard_MainLoop(&main_inst);
	std::cout << "Keyboard_MainLoop Finalizado\n";*/
	
	MelodyComp mc;

	BIT8* badApple_base = mc.compileFile("./melodies/BadApple.txt");
	VMMM::VirtualOrquesta vs;
	vs.setTrack(&main_inst, badApple_base, 50.0);
	vs.Concert_MainLoop();
	
	return 0;
}