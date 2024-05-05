#include <iostream>
#include "Virtual_Instrument.h"
#include "NoiseMachine.h"
#include "VitalComp.h"

#ifndef BIT8
#define BIT8 unsigned char
#endif

int main(int argc, char** argv) {
	Instrument_xml main_inst;
	if (0 > main_inst.load_document(argv[1])) {
		std::cout << "Instrument at" << argv[1] << " not found" << std::endl;
		return -1;
	}
	std::cout << "Instrument loaded succesfully" << std::endl;
	//"./melodies/BadApple.txt"
	if (argc == 2) {
		std::cout << "Loading Keyboard" << std::endl;
		VMMM::ConsoleSynth cs;
		cs.KeyBoard_MainLoop(&main_inst);
		std::cout << "Keyboard_MainLoop Finished\n";
		return 0;
	}
	MelodyComp mc;
	BIT8* melody = mc.compileFile(argv[2]);
	if (melody == NULL){
		std::cout << "Melody " << argv[2] << " not found" << std::endl;
		return 1;
	}
	VMMM::VirtualOrquesta vs;
	vs.setTrack(&main_inst, melody, 50.0);
	vs.Concert_MainLoop();
	
	return 0;
}