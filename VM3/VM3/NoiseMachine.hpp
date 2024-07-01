#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include <list>
#include <algorithm>
#include <iomanip>
#include <sstream>

#include "NoiseMaker.hpp"
#include "VirtualInstrument.hpp"

#ifndef FTYPE
#define FTYPE double
#endif

#ifndef BIT8
#define BIT8 unsigned char
#endif

namespace VMMM {
	class VMMM {
		protected: std::chrono::steady_clock::time_point clock_old_time, clock_real_time;
		protected: FTYPE dElapsedTime = 0, dWallTime = 0, dSaveTime = 0;
		protected: std::wstring curr_device;

		protected: static std::vector<Note> vecNotes;
		protected: static std::mutex muxNotes;
		protected: static std::string curr_output;

		protected: typedef bool(*lambda)(Note const& item);
		protected: template<class T> static void safe_remove(T& v, lambda f);

		protected: static FTYPE MakeNoise(int nChannel, FTYPE globalTime){
			std::unique_lock<std::mutex> lm(muxNotes);
			FTYPE dMixedOutput = 0.0;
			for (Note& n : vecNotes) {
				bool bNoteFinished = false;
				FTYPE dSound = 0;

				if (n.channel != nullptr)
					dSound = n.channel->getSound(globalTime, n, bNoteFinished);

				dMixedOutput += dSound;
				if (bNoteFinished)
					n.active = false;
			}
			safe_remove<std::vector<Note>>(vecNotes, [](Note const& item) { return item.active; });
			std::stringstream strstm;
			strstm << std::fixed << std::setprecision(2) << (dMixedOutput * 0.2);
			curr_output = strstm.str();
			return dMixedOutput * 0.5;
		}
		protected: olcNoiseMaker<short> sound;

		protected: void setTime(){
			this->clock_old_time = std::chrono::high_resolution_clock::now();
			this->clock_real_time = std::chrono::high_resolution_clock::now();
			this->dElapsedTime = 0.0;
			this->dWallTime = 0.0;
			this->dSaveTime = 0.0;
		}
		protected: void updateTime(){
			this->clock_real_time = std::chrono::high_resolution_clock::now();
			this->dElapsedTime = std::chrono::duration<FTYPE>(this->clock_real_time - this->clock_old_time).count();
			this->clock_old_time = this->clock_real_time;
			this->dWallTime += this->dElapsedTime;
			this->dSaveTime += this->dElapsedTime;
		}

		public: VMMM(){}
	};
	std::vector<Note> VMMM::vecNotes;
	std::mutex VMMM::muxNotes;
	std::string VMMM::curr_output;
	template<class T>
	void VMMM::safe_remove(T& t, lambda f) {
		auto n = t.begin();
		while (n != t.end())
			if (f(*n))//Nota activa
				++n;//La deja pasar
			else//Nota inactiva
				n = t.erase(n);//Es eliminada
	}

	class ConsoleSynth : public VMMM {
		private: bool ctl_right = true, ctl_left = true;
		private: char scale = 0;
		private: void print_board(){
			std::cout << "Virtual MusicMakerMachine V0.8.4 by JoGEHrt & OneLoneCoder" << std::endl << std::endl;
			std::wcout << "Device at Use: " << this->curr_device << std::endl;

			std::cout << std::endl <<
				"|ESC| = Exit" << std::endl << std::endl <<
				"<- Degrade Scale      Incriest Scale->" << std::endl <<
				"   C" << (int)(scale + 4)<< std::endl <<
				"|    |  S  |  D  |   |   |  G   |  H   |  J  |   |" << std::endl <<
				"|    | Do# | Re# |   |   | Fa#  | Sol# | La# |   |" << std::endl <<
				"|    |_____|_____|   |   |______|______|_____|   |" << std::endl <<
				"|   Z  |   X  |   C  |   V  |  B   |  N   |   M  |" << std::endl <<
				"| C/Do | D/Re | E/Mi | F/Fa | G/Sol| A/La | B/Si |" << std::endl <<
				"|______|______|______|______|______|______|______|" << std::endl << std::endl;
		}

		public: ConsoleSynth(){
			std::vector<std::wstring> devices = olcNoiseMaker<short>::Enumerate();
			for (std::wstring d : devices) std::wcout << "Output Device Found: " << d << std::endl;
			this->curr_device = devices[0];
		}
		public: void KeyBoard_MainLoop(Instrument_xml* inst){
			this->sound.Create(this->curr_device, 44100, 1, 8, 512);
			this->sound.SetUserFunction(this->MakeNoise);
			this->setTime();
			while (!GetAsyncKeyState(VK_ESCAPE)) {//Presione ESC para salir
				this->updateTime();
				FTYPE dTimeNow = sound.GetTime();

				if (ctl_right && GetAsyncKeyState(VK_RIGHT) & 0x8000) {
					scale = (scale + 1 <= 3 ? scale + 1 : -3);
					ctl_right = false;
				}
				else if (GetAsyncKeyState(VK_RIGHT) == 0)
					ctl_right = true;

				if (ctl_left && GetAsyncKeyState(VK_LEFT) & 0x8000) {
					scale = (scale - 1 >= -3 ? scale - 1 : 3);
					ctl_left = false;
				}
				else if (GetAsyncKeyState(VK_LEFT) == 0)
					ctl_left = true;

				for (int k = 0; k < 12; k++) {
					short nKeyState = GetAsyncKeyState((unsigned char)("ZSXDCVGBHNJM"[k]));
					this->muxNotes.lock();//std::find_if(this->vecNotes.begin(), this->vecNotes.end(), [&k](Note const& item) { return item.id == k + 2; });
					auto noteFound = std::find_if(this->vecNotes.begin(), this->vecNotes.end(), [&k](Note const& item) {return item.id == k + 2; });
					if (noteFound == this->vecNotes.end()) {  // Si no encuentra la nota en el vector
						if (nKeyState & 0x8000) { // Procede a crear la nota detectada
							Note n;
							n.id = k + 2;
							n.on = dTimeNow;
							n.scale = scale;
							n.channel = inst;//Instrumento a tocar
							n.active = true;
							this->vecNotes.emplace_back(n);
						}
					} else if (nKeyState & 0x8000) {// Si ha encontrado la nota y la tecla es presionada
						if ((*noteFound).off > (*noteFound).on) { //Si la nota cuenta con un final
							(*noteFound).on = dTimeNow; //Esta reiniciará el tiempo
							(*noteFound).off = 0.0;
							(*noteFound).active = true;
						}
					} else if ((*noteFound).off < (*noteFound).on) // Cuando es liberado, se le asigna este tiempo como fin
						(*noteFound).off = dTimeNow;

					this->muxNotes.unlock();
				}
				if (this->dSaveTime >= 1.0) {
					system("cls");
					this->print_board();
					std::cout << "Instrument at use: " << inst->getName() << std::endl;
					std::cout << "Simulated Notes: " << this->vecNotes.size() << std::endl;
					std::cout << "Execution Time: " << dTimeNow << std::endl;
					std::cout << "Current Value: " << this->curr_output << std::endl;
					this->dSaveTime -= 1.0;
				}
			}
			this->sound.Stop();
		}
	};

	typedef struct Track {
		public: Instrument_xml* inst;
		public: BIT8* melody;

		public: size_t currBeat, repeat, times;
		public: FTYPE tempo, accm_time, start, lifeTime;

		public: Track(Instrument_xml* inst, BIT8* melody, FTYPE tempo, FTYPE start, size_t repeat){
			this->inst = inst;
			this->melody = melody;
			this->currBeat = 0;
			this->tempo = tempo;
			this->start = start;
			this->accm_time = 0.0;
			this->repeat = repeat;
			this->times = 0;
			this->lifeTime = 0.0;
		}
		public: bool isActive(){
			if (this->repeat)
				return this->times < this->repeat;
			return true;
		}
		public: BIT8 getStatus(){
			return this->melody[currBeat * 3];
		}
		public: BIT8 getNote(){
			return this->melody[currBeat * 3 + 1];
		}
		public: BIT8 getTime(){
			return this->melody[currBeat * 3 + 2];
		}
		public: FTYPE getTimeSec(){
			return ((FTYPE)this->getTime() / 128.0) * (this->tempo / 60.0);
		}
		public: FTYPE getBeforeTimeSec(){
			if (this->currBeat == 0)
				return 0.0;
			return ((FTYPE)this->melody[(currBeat - 1) * 3 + 2] / 128.0) * (this->tempo / 60.0);
		}
		public: void nextBeat(){
			if (this->getStatus() != 0x7F)
				this->currBeat++;
		}
	};

	class VirtualOrquesta : public VMMM {
		private: std::vector<Track> tracks;
		private: std::vector<Note> curr_notes;

		public: VirtualOrquesta(){
			std::vector<std::wstring> devices = olcNoiseMaker<short>::Enumerate();
			for (std::wstring d : devices) std::wcout << "Output Device found: " << d << std::endl;
			this->curr_device = devices[0];
		}

		public: void setTrack(Instrument_xml* inst, BIT8* melody, FTYPE tempo, FTYPE start, size_t repeat){
			Track tr(inst, melody, tempo, start, repeat);
			this->tracks.push_back(tr);
		}
		public: void printInstTracks(){
			for (Track track : this->tracks)
				std::cout << track.inst->getName() << std::endl;
		}
		public: void popTrack(size_t index){
			if (index < this->tracks.size())
				this->tracks.erase(this->tracks.begin() + index);
		}

		public: size_t play(FTYPE dTime){
			this->curr_notes.clear();
			for (int i = 0; i < this->tracks.size(); i++) //Recorriendo Todas las pistas
				if (this->tracks[i].isActive()) {//Si esta se encuentra activa loops o no
					this->tracks[i].accm_time += dTime;//Se añade tiempo transcurrido
					while (this->tracks[i].accm_time > this->tracks[i].lifeTime) {//Mientras existan residuos del tiempo transcurrido
						this->tracks[i].lifeTime = this->tracks[i].getTimeSec();//Guardamos el último tiempo en caso de acorde, siempre vendrá la nota con el mayor tiempo primero
							do {
								if (this->tracks[i].getNote()) {//Si la nota no es de silencio
									Note n;//Debe construirse la nota
									n.channel = this->tracks[i].inst;
									n.active = true;
									n.id = (this->tracks[i].getNote() - 1) % 12;//De acuerdo a la nota tocada
									n.scale = (char)((this->tracks[i].getNote() - 1) / 12);
									n.off = this->tracks[i].getTimeSec(); //Le añadimos el tiempo a la nota final
									this->curr_notes.push_back(n);
								}
								this->tracks[i].nextBeat();//Avanzamos
							} while (this->tracks[i].getStatus() & 0x80);//Si el estado es de un Chord, almacenará las notas
							if (this->tracks[i].getStatus() == 0x7F) {//Cuando llegue al final de la melodía
								this->tracks[i].times++;//Señalamos el loop
								this->tracks[i].currBeat = 0;//Reiniciamos el beat actual
							}
							this->tracks[i].accm_time -= this->tracks[i].lifeTime;//Restamos el tiempo
					}
				} else this->popTrack(i--);//Sino está activa, se descarta.

			return this->curr_notes.size();//Retorna el tamaño de las notas.
		}
		public: void Concert_MainLoop(){
			this->sound.Create(this->curr_device, 44100, 1, 8, 512);
			this->sound.SetUserFunction(this->MakeNoise);
			this->setTime();
			while (this->tracks.size() && !GetAsyncKeyState(27)) {//Cuando no haya nada que tocar o no se presione ESC
				this->updateTime();
				size_t new_notes = this->play(this->dElapsedTime);
				this->muxNotes.lock();
				for (size_t i = 0; i < new_notes; i++) {
					this->curr_notes[i].on = dWallTime;//Se le añade el tiempo que debería estar activa
					this->curr_notes[i].off += dWallTime;
					vecNotes.emplace_back(this->curr_notes[i]);
				}
				this->muxNotes.unlock();
				if (this->dSaveTime >= 1.0) {
					system("cls");
					std::cout << "Simulated Notes: " << this->vecNotes.size() << std::endl;
					std::cout << "Execution Time: " << dWallTime << std::endl;
					this->dSaveTime -= 1.0;
				}
			}
			this->sound.Stop();
		}
	};
};
