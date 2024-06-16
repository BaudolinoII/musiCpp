#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include <list>
#include <algorithm>
#include <chrono>

#include "NoiseMaker.hpp"
#include "VirtualInstrument.hpp"
#include "NoiseMachine.hpp"

#ifndef FTYPE
#define FTYPE double
#endif

#ifndef BIT8
#define BIT8 unsigned char
#endif

VMMM::VMMM::VMMM(){}
FTYPE VMMM::VMMM::MakeNoise(int nChannel, FTYPE globalTime) {
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
	return dMixedOutput * 0.2;
}
void VMMM::VMMM::setTime() {
	this->clock_old_time = std::chrono::high_resolution_clock::now();
	this->clock_real_time = std::chrono::high_resolution_clock::now();
	this->dElapsedTime = 0.0;
	this->dWallTime = 0.0;
	this->dSaveTime = 0.0;
}
void VMMM::VMMM::updateTime() {
	this->clock_real_time = std::chrono::high_resolution_clock::now();
	this->dElapsedTime = std::chrono::duration<FTYPE>(this->clock_real_time - this->clock_old_time).count();
	this->clock_old_time = this->clock_real_time;
	this->dWallTime += this->dElapsedTime;
	this->dSaveTime += this->dElapsedTime;
}

VMMM::ConsoleSynth::ConsoleSynth() {
	std::vector<std::wstring> devices = olcNoiseMaker<short>::Enumerate();
	for (std::wstring d : devices) std::wcout << "Output Device Found: " << d << std::endl;
	this->curr_device = devices[0];
}
void VMMM::ConsoleSynth::print_board() {
	std::cout << "Virtual MusicMakerMachine V0.7.6 by JoGEHrt & OneLoneCoder" << std::endl << std::endl;
	std::wcout << "Device at Use: " << this->curr_device << std::endl;

	std::cout << std::endl <<
		"|ESC| = Exit" << std::endl << std::endl <<
		"<- Degrade Scale      Incriest Scale->" << std::endl <<
		"   C" << (int)scale << std::endl <<
		"|    |  S  |  D  |   |   |  G   |  H   |  J  |   |" << std::endl <<
		"|    | Do# | Re# |   |   | Fa#  | Sol# | La# |   |" << std::endl <<
		"|    |_____|_____|   |   |______|______|_____|   |" << std::endl <<
		"|   Z  |   X  |   C  |   V  |  B   |  N   |   M  |" << std::endl <<
		"| C/Do | D/Re | E/Mi | F/Fa | G/Sol| A/La | B/Si |" << std::endl <<
		"|______|______|______|______|______|______|______|" << std::endl << std::endl;
}
void VMMM::ConsoleSynth::KeyBoard_MainLoop(Instrument_xml* inst) {
	this->sound.Create(this->curr_device, 44100, 1, 8, 512);
	this->sound.SetUserFunction(this->MakeNoise);
	this->setTime();
	while (!GetAsyncKeyState(VK_ESCAPE)) {//Presione ESC para salir
		this->updateTime();
		FTYPE dTimeNow = sound.GetTime();

		if (ctl_right && GetAsyncKeyState(VK_RIGHT) & 0x8000) {
			scale = (scale + 1) % 3;
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
			this->muxNotes.lock();
			std::vector<Note>::iterator noteFound = find_if(this->vecNotes.begin(), this->vecNotes.end(), [&k](Note const& item) { return item.id == k + 2; });
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
			this->dSaveTime -= 1.0;
		}
	}
	this->sound.Stop();
}

VMMM::s_Track::s_Track(Instrument_xml* inst, BIT8* melody, FTYPE tempo, FTYPE start, size_t repeat) {
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
bool VMMM::s_Track::isActive() {
	if (this->repeat)
		return this->times < this->repeat;
	return true;
}
BIT8 VMMM::s_Track::getStatus() {
	return this->melody[currBeat * 3];
}
BIT8 VMMM::s_Track::getNote() {
	return this->melody[currBeat * 3 + 1];
}
BIT8 VMMM::s_Track::getTime() {
	return this->melody[currBeat * 3 + 2];
}
FTYPE VMMM::s_Track::getTimeSec() {
	return ((FTYPE)this->getTime() / 128.0) * (this->tempo / 60.0);
}
FTYPE VMMM::s_Track::getBeforeTimeSec() {
	if (this->currBeat == 0)
		return 0.0;
	return ((FTYPE)this->melody[(currBeat - 1) * 3 + 2] / 128.0) * (this->tempo / 60.0);
}
void VMMM::s_Track::nextBeat() {
	if (this->getStatus() != 0x7F)
		this->currBeat++;
}

VMMM::VirtualOrquesta::VirtualOrquesta() {
	std::vector<std::wstring> devices = olcNoiseMaker<short>::Enumerate();
	for (std::wstring d : devices) std::wcout << "Output Device found: " << d << std::endl;
	this->curr_device = devices[0];
}
void VMMM::VirtualOrquesta::setTrack(Instrument_xml* inst, BIT8* melody, FTYPE tempo, FTYPE start, size_t repeat) {
	Track tr(inst, melody, tempo, start, repeat);
	this->tracks.push_back(tr);
}
void VMMM::VirtualOrquesta::printInstTracks() {
	for (Track track : this->tracks)
		std::cout << track.inst->getName() << std::endl;
}
void VMMM::VirtualOrquesta::popTrack(size_t index) {
	if (index < this->tracks.size())
		this->tracks.erase(this->tracks.begin() + index);
}
size_t VMMM::VirtualOrquesta::play(FTYPE dTime) {
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
void VMMM::VirtualOrquesta::Concert_MainLoop() {
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