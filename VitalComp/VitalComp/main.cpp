#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include <list>
#include <algorithm>
#include <chrono>

#include "olcNoiseMaker.h"
#include "SoundTools.h"
#include "Virtual_Instruments.h"

#define FTYPE double

std::vector<stl::note> vecNotes;
std::mutex muxNotes;
inst::instrument_bell instBell;
inst::instrument_bell8 instBell8;
inst::instrument_harmonica instHarm;
inst::instrument_drumkick instKick;
inst::instrument_drumsnare instSnare;
inst::instrument_drumhihat instHiHat;

//Funcion para eliminar todas las notas inactivas o aquellas cuyo tiempo se ha agotado
typedef bool(*lambda)(stl::note const& item);
template<class T>
void safe_remove(T& v, lambda f){
	auto n = v.begin();
	while (n != v.end())
		if (f(*n))//Nota activa
			++n;//La deja pasar
		else//Nota inactiva
			n = v.erase(n);//Es eliminada
}

// Esta funcion es capaz de administrar la cantidad de instrumentos y notas interpretadas a la vez
FTYPE MakeNoise(int nChannel, FTYPE dTime) {
	std::unique_lock<std::mutex> lm(muxNotes);
	FTYPE dMixedOutput = 0.0;

	for (stl::note& n : vecNotes){
		bool bNoteFinished = false;
		FTYPE dSound = 0;

		// Reproduce cada nota con su correspondiente instrumento asignado
		if (n.channel != nullptr)
			dSound = n.channel->sound(dTime, n, bNoteFinished);

		// Adición a la mezcla del sonido
		dMixedOutput += dSound;

		if (bNoteFinished) //Si el tiempo sobrepasa la vida máxima o el tiempo activo a concluido
			n.active = false;//Entonces esta ya no está activa
	}

	safe_remove<std::vector<stl::note>>(vecNotes, [](stl::note const& item) { return item.active; });//Se descarta la nota del vector

	return dMixedOutput * 0.2;
}

int main() {
	std::wcout << "VitalComp V1" << std::endl << "Modo unico en Piano Sintetizador" << std::endl << std::endl;

	// Obtiene los dispositivos disponibles para reproducir audio
	std::vector<std::wstring> devices = olcNoiseMaker<short>::Enumerate();

	// Despliega la lista y usa la primera opción, por defecto son las bocinas de la computadora
	for (std::wstring d : devices) std::wcout << "Dispositivo de Salida Encontrado: " << d << std::endl;
	std::wcout << "Dispositivo en uso: " << devices[0] << std::endl;

	// Se imprime una visual del teclado
	std::wcout << std::endl <<
		"|   |   |   |   |   | |   |   |   |   | |   | |   |   |   |" << std::endl <<
		"|   | S |   |   | F | | G |   |   | J | | K | | L |   |   |" << std::endl <<
		"|   |___|   |   |___| |___|   |   |___| |___| |___|   |   |__" << std::endl <<
		"|     |     |     |     |     |     |     |     |     |     |" << std::endl <<
		"|  Z  |  X  |  C  |  V  |  B  |  N  |  M  |  ,  |  .  |  /  |" << std::endl <<
		"|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|" << std::endl << std::endl;

	// se construye la máquina de sonido con un muestreo de 44100, un canal, bloques de tamaño 8 y 512 en total
	olcNoiseMaker<short> sound(devices[0], 44100, 1, 8, 512);

	// Enlace de la función
	sound.SetUserFunction(MakeNoise);

	std::chrono::steady_clock::time_point clock_old_time = std::chrono::high_resolution_clock::now();
	std::chrono::steady_clock::time_point clock_real_time = std::chrono::high_resolution_clock::now();
	double dElapsedTime = 0.0;
	double dWallTime = 0.0;

	stl::VMMM seq(60.0);//VMMM y asignacionde instrumentos
	seq.AddInstrument(&instBell);
	seq.AddInstrument(&instBell8);
	seq.AddInstrument(&instHarm);

	seq.vecChannel.at(0).sBeat = "A...B...C..D.E..";//Secuencia de bits
	seq.vecChannel.at(1).sBeat = "..D...C...B...A.";
	seq.vecChannel.at(2).sBeat = "A.C.F.J.A.C.F.JA";

	while (1){
		// Actualizacion de tiempo =======================================================================================
		clock_real_time = std::chrono::high_resolution_clock::now();
		auto time_last_loop = clock_real_time - clock_old_time;
		clock_old_time = clock_real_time;
		dElapsedTime = std::chrono::duration<FTYPE>(time_last_loop).count();
		dWallTime += dElapsedTime;
		FTYPE dTimeNow = sound.GetTime();

		// Actualizacion de la VMMM (genera las notas y las añade al vector principal) ======================================
		int newNotes = seq.Update(dElapsedTime);
		muxNotes.lock();
		for (int a = 0; a < newNotes; a++){
			seq.vecNotes[a].on = dTimeNow;
			vecNotes.emplace_back(seq.vecNotes[a]);
		}
		muxNotes.unlock();

		for (int k = 0; k < 16; k++){
			short nKeyState = GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe\xbf"[k]));
			//double dTimeNow = sound.GetTime();

			// Para evitar 2 notas del mismo tipo, se revisa
			muxNotes.lock();
			std::vector<stl::note>::iterator noteFound = find_if(vecNotes.begin(), vecNotes.end(), [&k](stl::note const& item) { return item.id == k + 64 && item.channel == &instBell; });
			if (noteFound == vecNotes.end()){ // Si no encuentra la nota en el vector
				if (nKeyState & 0x8000){ // Procede a crear la nota detectada
					stl::note n;
					n.id = k + 64;
					n.on = dTimeNow;
					n.channel = &instBell;//Instrumento a tocar
					n.active = true;
					vecNotes.emplace_back(n);
				}
			} else { // Si la nota existe en el vector
				if (nKeyState & 0x8000){ // La nota está siendo presionada
					if (noteFound->off > noteFound->on){// Esta reiniciará su valor permaneciendo activa
						noteFound->on = dTimeNow;
						noteFound->active = true;
					}
				}else{ // Si ha dejado de presionarse, entonces marca su tiempo final para ser terminada sin un silencio directo
					if (noteFound->off < noteFound->on)
						noteFound->off = dTimeNow;
				}
			}
			muxNotes.unlock();
		}
		std::wcout << "\rNotas simultaneas: " << vecNotes.size() << "    ";
		//this_thread::sleep_for(5ms);
	}

	return 0;
}