#include <iostream>
#include "Virtual_Instrument.h"
#include "NoiseMachine.h"
#include "VitalComp.h"

#ifndef BIT8
#define BIT8 unsigned char
#endif

//San Francisco (Be Sure to Wear Some Flowers in Your Hair)
//By Scott McKenzie
//Size = 3n + 1	//Status, Note, Tempo
BIT8* SanFrancisco = new BIT8[133]{ 
	SILEN,SILEN,NG,
	SINGLE,SOL,NG,
	SINGLE,SOL,NG,//If
	SINGLE,SOL,NG,//You're
	SINGLE,MI,NG, //Go -
	SINGLE,MI,BL, //ing
	SINGLE,MI,NG, //to
	SINGLE,RE,BL, //San
	SINGLE,MI,BL, //Fran -
	SINGLE,SI,BL, //cis -
	SINGLE,LA,BL | NG, //co,
	SILEN,SILEN,NG,
	SINGLE,SOL,NG,//be
	SINGLE,SOL,NG,//sure
	SINGLE,SOL,NG,//to
	SINGLE,MI,BL | NG, //wear
	SINGLE,MI,NG, //some
	SINGLE,RE,NG, //flow
	SINGLE,MI,NG, //ers
	SINGLE,RE,NG, //in
	SINGLE,SI,NG, //your
	SINGLE,LA,RD, //hair
	SILEN,SILEN,BL,
	SINGLE,SOL,NG, //If
	SINGLE,LA,CH, //you're
	SINGLE,SOL,CH,
	SINGLE,RE,NG, //go -
	SINGLE,SI,BL, //ing
	SINGLE,LA,NG, //to
	SINGLE,SOL,BL, //San
	SINGLE,SOL,BL, //Fran-
	SINGLE,MI,NG, //cis -
	SINGLE,RE,BL | NG, //co
	SILEN,SILEN,NG,
	SINGLE,SOL,NG, //you're
	SINGLE,SOL,NG, //gon
	SINGLE,LA,NG, //na
	SINGLE,SI,BL | NG, //meet
	SINGLE,MI,NG, //some
	SINGLE,RE,CH, //gen -
	SINGLE,MI,CH, //tle
	SINGLE,RE,BL, //peo -
	SINGLE,SI,NG, //ple
	SILEN,SILEN,RD,
	FIN };

int main() {
	Instrument_xml inst;
	if (0 > inst.load_document("./xml_samples/Experimental2.xml")) {
		std::cout << "No se encontró el archivo" << std::endl;
		return -1;
	}
	std::cout << "Instrumento Cargado\n";

	/*VMMM::ConsoleSynth cs;
	cs.KeyBoard_MainLoop(&inst);
	std::cout << "Keyboard_MainLoop Finalizado\n";*/
	
	MelodyComp mc;
	size_t size;
	BIT8* test = mc.vitalComp("Do Do# Re Re# Mi_16 Fa.3 <Solb Sol La_1> La# Si silen", size);

	VMMM::VirtualOrquesta vs;
	vs.setTrack(&inst, SanFrancisco, 120.0);
	vs.Concert_MainLoop();
	return 0;
}