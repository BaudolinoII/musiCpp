#include <iostream>
#include "Virtual_Instrument.h"
#include "NoiseMachine.h"
#include "VitalComp.h"

#ifndef BIT8
#define BIT8 unsigned char
#endif




int main() {
	Instrument_xml main_inst, perq;
	if (0 > main_inst.load_document("./xml_samples/Experimental2.xml")) {
		std::cout << "No se encontro el archivo" << std::endl;
		return -1;
	}
	if (0 > perq.load_document("./xml_samples/Experimental.xml")) {
		std::cout << "No se encontro el archivo" << std::endl;
		return -1;
	}
	std::cout << "Instrumentos Cargados\n";

	/*VMMM::ConsoleSynth cs;
	cs.KeyBoard_MainLoop(&inst);
	std::cout << "Keyboard_MainLoop Finalizado\n";*/
	
	MelodyComp mc;
	/*Base de BAD APPLE [TOUHOU PROYECT] by ZUN*/
	std::string partiture = "Re# Fa Fa# Sol# La#_2 "
							"+Re# +Do# La#_2 Re#_2 "
							"La# Sol# Fa# Fa "

							"Re# Fa Fa# Sol# La#_2 "
							"Sol# Fa# Fa Re# Fa Fa# "
							"Fa Re# Re Fa , "

							"Re# Fa Fa# Sol# La#_2 "
							"+Re# +Do# La#_2 Re#_2 "
							"La# Sol# Fa# Fa "

							"Re# Fa Fa# Sol# La#_2 "
							"Sol# Fa# Fa Fa# Sol# La# , "

							"Re# Fa Fa# Sol# La#_2 "
							"+Re# +Do# La#_2 Re#_2 "
							"La# Sol# Fa# Fa "

							"Re# Fa Fa# Sol# La#_2 "
							"Sol# Fa# Fa silen Fa#_8 silen Sol#_8 "
							"silen La#_2 <Re#_2 +Re#_2>";
	//San Francisco (Be Sure to Wear Some Flowers in Your Hair)
	//By Scott McKenzie
	std::string partiture_right =
		"silen_2 Sol Sol "
		"+Mi.8 +Mi_2 +Mi_8"
		"+Re_2 +Re_2 "
		"Si_8 La_2"
		"silen_2 Sol Sol Sol_8 "
		;
	std::string partiture_left =
		"Mi_2 Mi Re "
		"+Do_2 +Do_2 "
		"Sol_2 Sol_2 "
		"Re_2 Fa#_8 Sol La "
		"Re_8 Mi_2 Mi Re"
		;
	BIT8* badApple_base = mc.vitalComp(partiture);
	BIT8* sanFrancisco_right = mc.vitalComp(partiture_right, MelodyComp::C3);
	BIT8* sanFrancisco_left = mc.vitalComp(partiture_left, MelodyComp::C2);
	VMMM::VirtualOrquesta vs;
	vs.setTrack(&main_inst, badApple_base, 50.0);
	vs.setTrack(&perq, badApple_base, 50.0);
	//vs.setTrack(&main_inst, sanFrancisco_right, 90.0);
	//vs.setTrack(&perq, sanFrancisco_left, 90.0);
	vs.Concert_MainLoop();
	return 0;
}