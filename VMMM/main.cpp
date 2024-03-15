#include <iostream>
#include "Virtual_Instrument.h"
#include "NoiseMachine.h"

int main() {
	Instrument_xml inst;
	if (0 > inst.load_document("./xml_samples/Experimental2.xml")) {
		std::cout << "No se encontró el archivo" << std::endl;
		return -1;
	}
	std::cout << "Instrumento Cargado\n";
	VMMM vm3;
	vm3.setInstrument(&inst);
	vm3.KeyBoard_MainLoop();
	return 0;
}