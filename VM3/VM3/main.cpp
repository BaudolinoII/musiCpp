#include<iostream>
#include<string>
#include<vector>
#include<limits>

#include"NoiseMachine.hpp"
#include"VirtualInstrument.hpp"
#include"VitalComp.hpp"

const std::vector<std::string> EXTENDS = { "mpp","xml","cmp" };
const std::string version = "V 0.8.4";
std::string comp_file = "None";
std::string exit_format = "temp.cmp";
std::string main_inst = "None";
bool play = true, compile = true;


char getTypeOfDoc(char* path) {
	std::string str(path);
	size_t pos = str.find_last_of('.');
	if (pos == std::string::npos)
		return 0;
	std::string ext = str.substr(pos + 1);
	std::cout << " " << ext << " ";
	for (size_t i = 0; i < EXTENDS.size(); i++)
		if (!EXTENDS[i].compare(ext))
			return i + 1;
	return 0;
}
std::string getFileName(char* path) {
	std::string str(path);
	size_t begin = str.find_last_of('\\');
	begin = (begin == std::string::npos ? 0 : begin);
	size_t end = str.find_last_of('.');
	return str.substr(begin, end - begin);
}
std::string getFileName(std::string path) {
	size_t begin = path.find_last_of('\\');
	begin = (begin == std::string::npos ? 0 : begin);
	size_t end = path.find_last_of('.');
	return path.substr(begin, end - begin);
}
void printHelp(char case_of) {
	switch (case_of) {
	case 'I':
		std::cout
			<< "Theres several kind of start after gives the argument:" << std::endl
			<< "Case 0: In Case there's not any additional argument: the protocol shall we iniciate on te main menu" << std::endl
			<< "Case 1: If there's a file.mpp directs execute the file info as you load manually from the begin" << std::endl
			<< "Case 2: If there's only kind of file.xml instrument, will be iniciating the keyboard with the first one" << std::endl
			<< "Case 3: If there's at least one of each, the melody will be executed and the inst.xml as default" << std::endl
			<< "Any Invalid argument or not compatible file, will be interpreted as a Case 0" << std::endl;
		break;
	case 'O':
		std::cout
			<< "To specify a specific format of exit, instead of directly reproduce the sound, you could try:" << std::endl
			<< "-o \\path\\fileaudio.wav" << std::endl
			<< "By now, this current version is noly capable to render in the format of wave" << std::endl;
		break;
	case 'M':
		std::cout
			<< "By only compilation, you must:" << std::endl
			<< "-m \\path\\fileaudio.mpp -o \\path\\comp.cmp" << std::endl;
		break;
	case 'S':
		std::cout
			<< "Putting a file.xml instrument, you will inicialize in the Keyboard Mode" << std::endl
			<< "\\path\\fileaudio.mpp \\path\\inst.xml" << std::endl;
		break;
	case '\0':
	default:
		std::cout
			<< "Welcome to the Virtual MusicMakerMachine " << version << " by JoGEHrt & OneLoneCoder" << std::endl
			<< "This proyect was made to get another way to make music by a mathematical source of sound" << std::endl
			<< "designed to get integrated with other coding projects like videogames, movie reders or " << std::endl
			<< "stand alone music player." << std::endl << std::endl
			<< "To get specific help, please retype over the \"-h\" the follow characters(Case sensitive)" << std::endl
			<< "-hI -> input" << std::endl
			<< "-hO -> output" << std::endl
			<< "-hM -> melody file" << std::endl
			<< "-hS -> model sounds" << std::endl;;
	}
}
void mainMenu(int option) {
	VMMM::ConsoleSynth cs;
	VMMM::VirtualOrquesta vs;
	MelodyComp mc;
	Instrument_xml main_instrument;
	BIT8* melody = nullptr;
	bool loop = true, no_first_time = false;
	while (loop) {
		if (option == 0) {
			std::cout << "Virtual Music Maker Machine (VM3)" << version << " by JoGEHrt & OneLoneCoder" << std::endl << std::endl;
			std::cout << "===== Main Menu =====" << std::endl;
			std::cout << "1. Compile a .mpp file" << std::endl;
			std::cout << "2. Play a .cmp file" << std::endl;
			std::cout << "3. Load an instrument.xml file" << std::endl;
			std::cout << "4. Exit" << std::endl;
			std::cout << "Please select an option: ";
			std::cin >> option;
			if (std::cin.fail()) {
				std::cin.clear();
				std::cin.ignore();
				std::cout << "Entrada invalida. Por favor, ingrese un numero del 1 al 4." << std::endl;
				continue;
			}
			no_first_time = true;
		}
		system("cls");
		switch (option) {
		case 1:
			if (no_first_time) {
				std::cout << "Enter the path and name of the file.mpp: " << std::endl;
				std::getline(std::cin, comp_file);
			}
			std::cout << "Compile Archive " << comp_file << " to " << exit_format << std::endl;
			break;
		case 2:
			if (no_first_time) {
				std::cout << "Enter the path and name of the melody.mpp or .cmp: " << std::endl;
				std::getline(std::cin, exit_format);
			}
			melody = mc.compileFile(exit_format);
			if (0 > main_instrument.load_document(main_inst)) {
				std::cout << "Instrument at " << main_inst << " not found" << std::endl;
				break;
			}
			vs.setTrack(&main_instrument, melody, 60.0, 0.0, 1);
			vs.Concert_MainLoop();
			std::cout << "Playing " << exit_format << std::endl;
			break;
		case 3:
			if (no_first_time) {
				std::cout << "Enter the path and name of the instrument: " << std::endl;
				std::getline(std::cin, main_inst);
			}
			if (0 > main_instrument.load_document(main_inst)) {
				std::cout << "Instrument at " << main_inst << " not found" << std::endl;
				break;
			}
			std::cout << "KeyBoard Mode Active with " << main_inst << std::endl;
			cs.KeyBoard_MainLoop(&main_instrument);
			break;
		case 4:
			std::cout << "Exiting..." << std::endl;
			loop = false;
			break;
		default:
			std::cout << "Invalid option. Please select an option from 1 to 4." << std::endl;
			break;
		}
		option = 0;
		
	}

}

int main(int argc, char** argv) {
	for (size_t i = 1; i < argc; i++) {
		if (argv[i][0] == '-')
			switch (argv[i][1]) {
			case 'h':
				printHelp(argv[i][2]);
				break;
			case 'v':
				std::cout << "Virtual Music Maker Machine" << version << std::endl;
				break;
			case 'o':
				exit_format = argv[++i]; play = !play;
				break;
			}
		else switch (getTypeOfDoc(argv[i])) {
		case 1:
			comp_file = argv[i]; 
			break;
		case 2:
			main_inst = argv[i]; 
			break;
		case 3:
			play = true; compile = false; 
			break;
		}
	}
	if (!(comp_file.compare("None") || main_inst.compare("None"))) {
		mainMenu(0);
		return 0;
	}
	if (!comp_file.compare("None")) {
		mainMenu(3);
		return 0;
	}
	if (compile) {
		if (!exit_format.compare("temp.cmp"))
			exit_format = getFileName(comp_file) + ".cmp";
		mainMenu(1);
		return 0;
	}
	if (play) {
		mainMenu(2);
		return 0;
	}
	return 0;
}