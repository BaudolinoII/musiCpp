#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include <list>
#include <algorithm>
#include <chrono>

#include "olcNoiseMaker.h"
#include "Virtual_Instrument.h"

#ifndef FTYPE
#define FTYPE double
#endif

#ifndef BIT8
#define BIT8 unsigned char
#endif

const BIT8 MI = 0x41, FA = 0x42, FA_M = 0x43, SOL = 0x44, SOL_M = 0x45, LA = 0x46;
const BIT8 LA_M = 0x47, SI = 0x48, DO = 0x49, DO_M = 0x4A, RE = 0x4B, RE_M = 0x4C;
const BIT8 RD = 0x80, BL = 0x40, NG = 0x20, CH = 0x10, SH = 0x08;
const BIT8 SINGLE = 0x40, CHORD = 0xC0, SILEN = 0x00, FIN = 0xBF;

//stl::VMMM seq(30.0);//VMMM y asignacion de instrumentos
	//seq.setInstrument(&instHarm);
	//seq.AddInstrument(&instBell8);
	//seq.AddInstrument(&instHarm);

	/*									//San Francisco (Be Sure to Wear Some Flowers in Your Hair)
										//By Scott McKenzie
							//Size = 3n + 1	//Status, Note, Tempo
	seq.vecChannel.sBeat = new BIT8[133]{ SILEN,SILEN,NG,
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
	seq.bRepeat = true;
	*/

namespace admin {
	std::vector<Note> vecNotes;
	std::mutex muxNotes;
	Instrument_xml inst;
	typedef bool(*lambda)(Note const& item);
	template<class T>
	void safe_remove(T& v, lambda f) {
		auto n = v.begin();
		while (n != v.end())
			if (f(*n))//Nota activa
				++n;//La deja pasar
			else//Nota inactiva
				n = v.erase(n);//Es eliminada
	}
	FTYPE MakeNoise(int nChannel, FTYPE dTime) {
		std::unique_lock<std::mutex> lm(muxNotes);
		FTYPE dMixedOutput = 0.0;
		for (Note& n : vecNotes) {
			bool bNoteFinished = false;
			FTYPE dSound = 0;
			// Reproduce cada nota con su correspondiente instrumento asignado
			if (n.channel != nullptr)
				dSound = n.channel->getSound(dTime, n, bNoteFinished);
			// Adición a la mezcla del sonido
			dMixedOutput += dSound;
			if (bNoteFinished) //Si el tiempo sobrepasa la vida máxima o el tiempo activo a concluido
				n.active = false;//Entonces esta ya no está activa
		}
		safe_remove<std::vector<Note>>(vecNotes, [](Note const& item) { return item.active; });//Se descarta la nota del vector
		return dMixedOutput * 0.2;
	}
}

class VMMM {
	private: std::chrono::steady_clock::time_point clock_old_time, clock_real_time;
	private: olcNoiseMaker<short> sound;
	private: FTYPE dElapsedTime = 0, dWallTime = 0, dSaveTime = 0;

	private: void print_board() {
		std::wcout << std::endl <<
		"  | | S |   |   | F | | G |   |   | J | | K | | L |   |   |" << std::endl <<
		"  | |La#|   |   |Do#| |Re#|   |   |Fa#| |Sl#| |La#|   |   |" << std::endl <<
		"__| |___|   |   |___| |___|   |   |___| |___| |___|   |   |__" << std::endl <<
		"|   Z |   X |   C |   V |   B |  N  |  M  |  ;  |  :  |  _  |" << std::endl <<
		"|La 4°|  Si |  Do |  Re |  Mi |  Fa | Sol |La 5°|  Si |  Do |" << std::endl <<
		"|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|" << std::endl << std::endl;
	}
	private: void setTime(){
		this->clock_old_time = std::chrono::high_resolution_clock::now();
		this->clock_real_time = std::chrono::high_resolution_clock::now();
		this->dElapsedTime = 0.0;
		this->dWallTime = 0.0;
		this->dSaveTime = 0.0;
	}
	private: void updateTime(){
		this->clock_real_time = std::chrono::high_resolution_clock::now();
		this->dElapsedTime = std::chrono::duration<FTYPE>(this->clock_real_time - this->clock_old_time).count();
		this->clock_old_time = this->clock_real_time;
		this->dWallTime += this->dElapsedTime;
		this->dSaveTime += this->dElapsedTime;
	}
	
	public: VMMM(){
	}
	public: void init() {
		std::wcout << "VitalMusicMakerMachine V0.1.2 by JoGEHrt & OneLoneCoder" << std::endl << std::endl;

		std::vector<std::wstring> devices = olcNoiseMaker<short>::Enumerate();
		for (std::wstring d : devices) std::wcout << "Dispositivo de Salida Encontrado: " << d << std::endl;
		std::wcout << "Dispositivo en uso: " << devices[0] << std::endl;

		this->sound.Create(devices[0], 44100, 1, 8, 512);

		sound.SetUserFunction(admin::MakeNoise);
	}
	// Actualizacion de la VMMM (genera las notas y las añade al vector principal) ======================================
	//int newNotes = seq.Update(dElapsedTime);
	//muxNotes.lock();
	//for (int a = 0; a < newNotes; a++) {
		//seq.vecNotes[a].on = dTimeNow;//Se le añade el tiempo que debería estar activa
		//vecNotes.emplace_back(seq.vecNotes[a]);
	//}
	//admin::muxNotes.unlock();
	public: void KeyBoard_MainLoop(Instrument_xml *inst) {
		this->setTime();
		while (1) {
			this->updateTime();
			FTYPE dTimeNow = this->sound.GetTime();
			for (int k = 0; k < 16; k++) {
				short nKeyState = GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe\xbf"[k]));
				// Para evitar 2 notas del mismo tipo, se revisa
				admin::muxNotes.lock();
				std::vector<Note>::iterator noteFound = find_if(admin::vecNotes.begin(), admin::vecNotes.end(), [&k](Note const& item) { return item.id == k + 70; });
				if (noteFound == admin::vecNotes.end()) { // Si no encuentra la nota en el vector
					if (nKeyState & 0x8000) { // Procede a crear la nota detectada
						Note n;
						n.id = k + 70;
						n.on = dTimeNow;
						n.channel = inst;//Instrumento a tocar
						n.active = true;
						admin::vecNotes.emplace_back(n);
					}
				}
				else { // Si la nota existe en el vector
					if (nKeyState & 0x8000) { // La nota está siendo presionada
						if (noteFound->off > noteFound->on) {//En caso de Ya haberse asignado un final
							noteFound->on = dTimeNow;// Esta reiniciará su valor permaneciendo activa
							//noteFound->off = 0.0;//Se vuelve cero para sostenerse
							noteFound->active = true;
						}
					}
					else { // Si ha dejado de presionarse, entonces marca su tiempo final para ser terminada sin un silencio directo
						//Tambien asigna un tiempo final al vector de notas generadas
						//if (noteFound->off < noteFound->on)
						noteFound->off = dTimeNow;//Le suma el tiempo de vida que carga la nota
					}
				}
				admin::muxNotes.unlock();
			}
			
			if (this->dSaveTime >= 1.0) {
				system("cls");
				this->print_board();
				std::cout << "Notas simultaneas: " << admin::vecNotes.size() << std::endl;
				std::cout << "Tiempo Transcurrido: " << dWallTime << std::endl;
				std::cout << "dTimeNow: " << dTimeNow << std::endl;
				this->dSaveTime -= 1.0;
			}
			
			//this_thread::sleep_for(5ms);
		}
	}
};
