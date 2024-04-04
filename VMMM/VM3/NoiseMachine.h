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

namespace VMMM{
	class VMMM {
		protected: std::chrono::steady_clock::time_point clock_old_time, clock_real_time;
		protected: FTYPE dElapsedTime = 0, dWallTime = 0, dSaveTime = 0;
		protected: std::wstring curr_device;

		protected: static std::vector<Note> vecNotes;
		protected: static std::mutex muxNotes;
		
		protected: typedef bool(*lambda)(Note const& item);
		protected: template<class T>
				static void safe_remove(T& v, lambda f) {
				auto n = v.begin();
				while (n != v.end())
					if (f(*n))//Nota activa
						++n;//La deja pasar
					else//Nota inactiva
						n = v.erase(n);//Es eliminada
			}
		protected: static FTYPE MakeNoise(int nChannel, FTYPE dTime) {
				std::unique_lock<std::mutex> lm(muxNotes);
				FTYPE dMixedOutput = 0.0;
				for (Note& n : vecNotes) {
					bool bNoteFinished = false;
					FTYPE dSound = 0;

					if (n.channel != nullptr)
						dSound = n.channel->getSound(dTime, n, bNoteFinished);

					dMixedOutput += dSound;
					if (bNoteFinished) 
						n.active = false;
				}
				safe_remove<std::vector<Note>>(vecNotes, [](Note const& item) { return item.active; });
				return dMixedOutput * 0.2;
			}
		protected: olcNoiseMaker<short> sound;
		
		protected: void setTime() {
			this->clock_old_time = std::chrono::high_resolution_clock::now();
			this->clock_real_time = std::chrono::high_resolution_clock::now();
			this->dElapsedTime = 0.0;
			this->dWallTime = 0.0;
			this->dSaveTime = 0.0;
		}
		protected: void updateTime() {
			this->clock_real_time = std::chrono::high_resolution_clock::now();
			this->dElapsedTime = std::chrono::duration<FTYPE>(this->clock_real_time - this->clock_old_time).count();
			this->clock_old_time = this->clock_real_time;
			this->dWallTime += this->dElapsedTime;
			this->dSaveTime += this->dElapsedTime;
		}
	};
	std::vector<Note> VMMM::vecNotes;
	std::mutex VMMM::muxNotes;

	class ConsoleSynth : public VMMM{
		private: void print_board() {
			std::cout << "VitalMusicMakerMachine V0.4.5 by JoGEHrt & OneLoneCoder" << std::endl << std::endl;
			std::wcout << "Dispositivo en uso: " << this->curr_device << std::endl;
			
			std::cout << std::endl <<
				"|ESC| = exit" << std::endl << std::endl <<
				"  | |La#|   |   |Do#| |Re#|   |   |Fa#| |Sl#| |La#|   |   |" << std::endl <<
				"__| |___|   |   |___| |___|   |   |___| |___| |___|   |   |__" << std::endl <<
				"|   Z |   X |   C |   V |   B |  N  |  M  |  ;  |  :  |  _  |" << std::endl <<
				"|La 4°|  Si |  Do |  Re |  Mi |  Fa | Sol |La 5°|  Si |  Do |" << std::endl <<
				"|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|" << std::endl << std::endl;
		}
		
		public: ConsoleSynth() {
			std::vector<std::wstring> devices = olcNoiseMaker<short>::Enumerate();
			for (std::wstring d : devices) std::wcout << "Dispositivo de Salida Encontrado: " << d << std::endl;
			this->curr_device = devices[0];
		}

		public: void KeyBoard_MainLoop(Instrument_xml* inst) {
			this->sound.Create(this->curr_device, 44100, 1, 8, 512);
			this->sound.SetUserFunction(this->MakeNoise);
			this->setTime();
			while (!GetAsyncKeyState(27)) {//Presione ESC para salir
				this->updateTime();
				FTYPE dTimeNow = sound.GetTime();
				for (int k = 0; k < 16; k++) {
					short nKeyState = GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe\xbf"[k]));
					// Para evitar 2 notas del mismo tipo, se revisa
					this->muxNotes.lock();
					std::vector<Note>::iterator noteFound = find_if(this->vecNotes.begin(), this->vecNotes.end(), [&k](Note const& item) { return item.id == k + 6; });
					if (noteFound == this->vecNotes.end())  // Si no encuentra la nota en el vector
						if (nKeyState & 0x8000) { // Procede a crear la nota detectada
							Note n;
							n.id = k + 6;
							n.on = dTimeNow;
							n.channel = inst;//Instrumento a tocar
							n.active = true;
							this->vecNotes.emplace_back(n);
						}
					else if (nKeyState & 0x8000)  // La nota está siendo presionada
						if (noteFound->off > noteFound->on) {//En caso de Ya haberse asignado un final
							noteFound->on = dTimeNow;// Esta reiniciará su valor permaneciendo activa
							noteFound->off = 0.0;//Se vuelve cero para sostenerse
							noteFound->active = true;
						}
					else noteFound->off = dTimeNow;//Le suma el tiempo de vida que carga la nota
						
					
					this->muxNotes.unlock();
				}

				if (this->dSaveTime >= 1.0) {
					system("cls");
					this->print_board();
					std::cout << "Instrumento en uso: " << inst->getName() << std::endl;
					std::cout << "Notas simultaneas: " << this->vecNotes.size() << std::endl;
					std::cout << "Tiempo de simulacion: " << dTimeNow << std::endl;
					this->dSaveTime -= 1.0;
				}
			}
			this->sound.Stop();
		}
	};
	
	typedef struct s_Track {
		public: Instrument_xml* inst;
		public: BIT8* melody;

		public: size_t currBeat, repeat, times;
		public: FTYPE tempo, accm_time, start, lifeTime;

		public: s_Track(Instrument_xml* inst, BIT8* melody, FTYPE tempo, FTYPE start, size_t repeat) {
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
		public: bool isActive() {
			if (this->repeat)
				return this->times < this->repeat;
			return true;
		}
		public: BIT8 getStatus() {
			return this->melody[currBeat * 3];
		}
		public: BIT8 getNote() {
			return this->melody[currBeat * 3 + 1];
		}
		public: BIT8 getTime() {
			return this->melody[currBeat * 3 + 2];
		}
		public: FTYPE getTimeSec() {
			return ((FTYPE)this->getTime() / 128.0) * (this->tempo / 60.0);
		}
		public: FTYPE getBeforeTimeSec() {
			if (this->currBeat == 0)
				return 0.0;
			return ((FTYPE)this->melody[(currBeat - 1) * 3 + 2] / 128.0) * (this->tempo / 60.0);
		}
		public: void nextBeat() {
			if (this->getStatus() != 0x7F)
				this->currBeat++;
		}
	}Track;

	class VirtualOrquesta : public VMMM{
		private: std::vector<Track> tracks;
		private: std::vector<Note> curr_notes;
		
		public: VirtualOrquesta(){
			std::vector<std::wstring> devices = olcNoiseMaker<short>::Enumerate();
			for (std::wstring d : devices) std::wcout << "Dispositivo de Salida Encontrado: " << d << std::endl;
			this->curr_device = devices[0];
		}

		public: void setTrack(Instrument_xml* inst, BIT8* melody, FTYPE tempo = 60.0, FTYPE start = 0.0, size_t repeat = 1){
			Track tr(inst, melody, tempo, start, repeat);
			this->tracks.push_back(tr);
		}
		public: void printInstTracks(){
			for (Track track : this->tracks)
				std::cout << track.inst->getName() << std::endl;
		}
		public: void popTrack(size_t index) {
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
							if (this->tracks[i].getStatus() & 0x40) {//Si el estado no es de silencio
								Note n;//Debe construirse la nota
								n.channel = this->tracks[i].inst;
								n.active = true;
								n.id = this->tracks[i].getNote();//De acuerdo a la nota tocada
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
				}
				else this->popTrack(i--);//Sino está activa, se descarta.
			
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
					std::cout << "Notas simultaneas: " << this->vecNotes.size() << std::endl;
					std::cout << "Tiempo de simulacion: " << dWallTime << std::endl;
					this->dSaveTime -= 1.0;
				}
			}
			this->sound.Stop();
		}
	};
};

