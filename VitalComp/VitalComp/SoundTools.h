#pragma once
#include<cmath>

#ifndef FTYPE
#define FTYPE double
#endif

namespace stl {//Sound Tools: Herramientas de sonido
	const size_t OSC_SINE = 0, OSC_SQUARE = 1, OSC_TRIANGLE = 2, OSC_SAW_ANA = 3, OSC_SAW_DIG = 4, OSC_NOISE = 5;
	const FTYPE PI = 2.0 * std::acos(0.0);
	const size_t SCALE_DEFAULT = 0;

	FTYPE scale(const int nNoteID, const int nScaleID = SCALE_DEFAULT){
		switch (nScaleID){
			case SCALE_DEFAULT: 
			default:
				return 8 * pow(1.0594630943592952645618252949463, nNoteID);
		}
	}
	FTYPE w(FTYPE dHertz) {
		return dHertz * 2.0 * PI;
	}
	FTYPE osc(const FTYPE dTime, const FTYPE dHertz, const int nType = OSC_SINE, const FTYPE dLFOHertz = 0.0, const FTYPE dLFOAmplitude = 0.0, FTYPE dCustom = 50.0){
		FTYPE dFreq = w(dHertz) * dTime + dLFOAmplitude * dHertz * (sin(w(dLFOHertz) * dTime));// osc(dTime, dLFOHertz, OSC_SINE);
		switch (nType){
		case OSC_SINE: // Sine wave bewteen -1 and +1
			return sin(dFreq);
		case OSC_SQUARE: // Square wave between -1 and +1
			return sin(dFreq) > 0 ? 1.0 : -1.0;
		case OSC_TRIANGLE: // Triangle wave between -1 and +1
			return asin(sin(dFreq)) * (2.0 / PI);
		case OSC_SAW_ANA: {// Saw wave (analogue / warm / slow)
			FTYPE dOutput = 0.0;
			for (FTYPE n = 1.0; n < dCustom; n++)
				dOutput += (sin(n * dFreq)) / n;
			return dOutput * (2.0 / PI);
		}
		case OSC_SAW_DIG:
			return (2.0 / PI) * (dHertz * PI * fmod(dTime, 1.0 / dHertz) - (PI / 2.0));
		case OSC_NOISE:
			return 2.0 * ((FTYPE)rand() / (FTYPE)RAND_MAX) - 1.0;
		default:
			return 0.0;
		}
	}

	class instrument_base;

	typedef struct sNote {
		BYTE id, status, timing; // Posicion en la nota y estado de esta y el tiempo representado por Redondas, Blancas, Negras, etc
		FTYPE on, off;			 // Tiempo de nota activa y cuando debe acabar (Esta última será relevante cuando automaticemos el proceso)
		bool active;			 // Bandera de activacion para ser descartada en el vector principal
		instrument_base* channel;//Instrumento Designado

		sNote() {
			id = 0;
			status = 0;
			/*
			Significado de los Bits de Status
			7 := si es 1, El generador de notas debe esperar a otras notas  hasta recibir un estatus de 0
			6 := si es 1, Esta nota corresponde a un silencio ignora el tono y solo aumenta la cuenta del tiempo
			5 := si es 1, Indica que esta última nota sobrepasó los ticks asignados a la frase ó es la última de esta y debe sonar distintiva
			4 := si es 1, Indica que esta nota tiene una duración de 1.5 veces su valor asignado, si bien esta será reflejada en el byte de tiempo, se debe indicar para el instrumento
			[3 - 0] := estos bits corresponden a los estados que puede adoptar cierta nota en determinado instrumento*/
			timing = 0x04;
			on = 0.0;
			off = 0.0;
			active = false;
			channel = nullptr;
		}
	} note;
    typedef struct sChannel {
		instrument_base* instrument;
		std::string sBeat;
	}channel;
	class templateNote{ 
		virtual FTYPE GetAmplitude(const FTYPE dTime, const FTYPE dTimeOn, const FTYPE dTimeOff) = 0;
	};
	class templateADSR : public templateNote {
		public: FTYPE dAttackTime, dDecayTime, dSustainAmplitude, dReleaseTime, dStartAmplitude;
		public: templateADSR() {
			this->dAttackTime = 0.10;
			this->dDecayTime = 0.01;
			this->dStartAmplitude = 1.0;
			this->dSustainAmplitude = 0.8;
			this->dReleaseTime = 0.20;
		}
		public: FTYPE GetAmplitude(const FTYPE dTime, const FTYPE dTimeOn, const FTYPE dTimeOff) {
			FTYPE dAmplitude = 0.0;
			FTYPE dReleaseAmplitude = 0.0;

			if (dTimeOn > dTimeOff){ // Note is on
				FTYPE dLifeTime = dTime - dTimeOn;

				if (dLifeTime <= dAttackTime)
					dAmplitude = (dLifeTime / dAttackTime) * dStartAmplitude;

				if (dLifeTime > dAttackTime && dLifeTime <= (dAttackTime + dDecayTime))
					dAmplitude = ((dLifeTime - dAttackTime) / dDecayTime) * (dSustainAmplitude - dStartAmplitude) + dStartAmplitude;

				if (dLifeTime > (dAttackTime + dDecayTime))
					dAmplitude = dSustainAmplitude;
			}else{ // Note is off
			
				FTYPE dLifeTime = dTimeOff - dTimeOn;

				if (dLifeTime <= dAttackTime)
					dReleaseAmplitude = (dLifeTime / dAttackTime) * dStartAmplitude;

				if (dLifeTime > dAttackTime && dLifeTime <= (dAttackTime + dDecayTime))
					dReleaseAmplitude = ((dLifeTime - dAttackTime) / dDecayTime) * (dSustainAmplitude - dStartAmplitude) + dStartAmplitude;

				if (dLifeTime > (dAttackTime + dDecayTime))
					dReleaseAmplitude = dSustainAmplitude;

				dAmplitude = ((dTime - dTimeOff) / dReleaseTime) * (0.0 - dReleaseAmplitude) + dReleaseAmplitude;
			}

			// Amplitude should not be negative
			if (dAmplitude <= 0.01)
				dAmplitude = 0.0;

			return dAmplitude;
		}
	};

	class instrument_base {
		protected: std::wstring name;
		protected: FTYPE dVolume, fMaxLifeTime;
		protected: stl::templateADSR tmp;
		public: virtual FTYPE sound(const FTYPE dTime, stl::note n, bool& bNoteFinished) = 0;
	};
	class VMMM{ //Virtual Machine Music Maker
		
		public: size_t nBeats, nSubBeats, nCurrentBeat, nTotalBeats;
		public: FTYPE fTempo, fBeatTime, fAccumulate;
		public: std::vector<channel> vecChannel;
		public: std::vector<note> vecNotes;

		public: VMMM(FTYPE tempo = 120.0f, size_t beats = 4, size_t subbeats = 4){
			nBeats = beats;
			nSubBeats = subbeats;
			fTempo = tempo;
			fBeatTime = (60.0f / fTempo) / (float)nSubBeats;
			nCurrentBeat = 0;
			nTotalBeats = nSubBeats * nBeats;
			fAccumulate = 0;
		}

		public: size_t Update(FTYPE fElapsedTime){//Proceso genera notas de acuerdo al tiempo real
			vecNotes.clear();//Se vacía el vector para incluir las nuevas notas
			fAccumulate += fElapsedTime;//Se acumula el tiempo transcurrido
			while (fAccumulate >= fBeatTime){//Si el tiempo es mayor o igual al espacio entre Beats, entonces procede
				fAccumulate -= fBeatTime;//Se decrementa el tiempo para compensar, en caso de que exista un retraso
				nCurrentBeat++;//Recorre el beat actual
				if (nCurrentBeat >= nTotalBeats)//Verifica si el beat está en sus totales
					nCurrentBeat = 0;//Se reinica en dado caso, puede simplificarse con una operación de módulo

				for (channel v : vecChannel)//Por cada instrumento o canal contenido
					if (v.sBeat[nCurrentBeat] != '.'){//Si la nota se encuentra en tal estado
						note n;//Debe construirse la nota
						n.channel = v.instrument;
						n.active = true;
						n.id = v.sBeat[nCurrentBeat] - 1;//De acuerdo a la nota tocada
						vecNotes.push_back(n);
					}
				
			}
			return vecNotes.size();
		}

		void AddInstrument(instrument_base* inst){
			channel c;
			c.instrument = inst;
			vecChannel.push_back(c);
		}

	};
};