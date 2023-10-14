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
inst::instrument_harmonica instHarm;

//Funcion para eliminar todas las notas inactivas o aquellas cuyo tiempo se ha agotado
typedef bool(*lambda)(stl::note const& item);
template<class T>
void safe_remove(T& v, lambda f){
	auto n = v.begin();
	while (n != v.end())
		if (!f(*n))
			n = v.erase(n);
		else
			++n;
}

// Esta funcion es capaz de administrar la cantidad de instrumentos y notas interpretadas a la vez
FTYPE MakeNoise(int nChannel, FTYPE dTime) {
	std::unique_lock<std::mutex> lm(muxNotes);
	FTYPE dMixedOutput = 0.0;

	for (stl::note& n : vecNotes){
		bool bNoteFinished = false;
		FTYPE dSound = 0;
		if (n.channel == 2)
			dSound = instBell.sound(dTime, n, bNoteFinished);
		if (n.channel == 1)
			dSound = instHarm.sound(dTime, n, bNoteFinished) * 0.5;
		dMixedOutput += dSound;

		if (bNoteFinished && n.off > n.on)
			n.active = false;
	}

	safe_remove<std::vector<stl::note>>(vecNotes, [](stl::note const& item) { return item.active; });

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

	char keyboard[129];
	memset(keyboard, ' ', 127);
	keyboard[128] = '\0';

	auto clock_old_time = std::chrono::high_resolution_clock::now();
	auto clock_real_time = std::chrono::high_resolution_clock::now();
	double dElapsedTime = 0.0;

	while (1){
		for (int k = 0; k < 16; k++){
			short nKeyState = GetAsyncKeyState((unsigned char)("ZSXCFVGBNJMK\xbcL\xbe\xbf"[k]));
			double dTimeNow = sound.GetTime();

			// Para evitar 2 notas del mismo tipo, se revisa
			muxNotes.lock();
			std::vector<stl::note>::iterator noteFound = find_if(vecNotes.begin(), vecNotes.end(), [&k](stl::note const& item) { return item.id == k; });
			if (noteFound == vecNotes.end()){ // Si no encuentra la nota en el vector
				if (nKeyState & 0x8000){ // Procede a crear la nota detectada
					stl::note n;
					n.id = k;
					n.on = dTimeNow;
					n.channel = 1;
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
					if (noteFound->off < noteFound->on){
						noteFound->off = dTimeNow;
					}
				}
			}
			muxNotes.unlock();
		}
		std::wcout << "\rNotas simultaneas: " << vecNotes.size() << "    ";
		//this_thread::sleep_for(5ms);
	}

	return 0;
}