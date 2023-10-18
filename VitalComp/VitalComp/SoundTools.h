#pragma once
#include<cmath>

#ifndef FTYPE
#define FTYPE double
#endif

namespace stl {//Sound Tools: Herramientas de sonido
	const size_t OSC_SINE = 0, OSC_SQUARE = 1, OSC_TRIANGLE = 2, OSC_SAW_ANA = 3, OSC_SAW_DIG = 4, OSC_NOISE = 5;
	const FTYPE PI = 2.0 * std::acos(0.0);
	const size_t SCALE_DEFAULT = 0;

	FTYPE scale(const int nNoteID, const int nScaleID = SCALE_DEFAULT) {
		switch (nScaleID) {
		case SCALE_DEFAULT:
		default:
			return 8 * pow(1.0594630943592952645618252949463, nNoteID);
		}
	}
	FTYPE w(FTYPE dHertz) {
		return dHertz * 2.0 * PI;
	}
	FTYPE osc(const FTYPE dTime, const FTYPE dHertz, const int nType = OSC_SINE, const FTYPE dLFOHertz = 0.0, const FTYPE dLFOAmplitude = 0.0, FTYPE dCustom = 50.0) {
		FTYPE dFreq = w(dHertz) * dTime + dLFOAmplitude * dHertz * (sin(w(dLFOHertz) * dTime));// osc(dTime, dLFOHertz, OSC_SINE);
		switch (nType) {
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
		FTYPE on, off, life;	 // Tiempo de nota presionada y liberada; Tiempo que permanecerá viva en generacion automática
		bool active;			 // Bandera de activacion para ser descartada en el vector principal
		instrument_base* channel;//Instrumento Designado

		sNote() {
			id = 0;
			status = 0;
			/*Significado de los Bits de Status
			7 := si es 1, El generador de notas debe esperar a otras notas  hasta recibir un estatus de 0 :: 0xF0
			6 := si es 0, Esta nota corresponde a un silencio ignora el tono y solo aumenta la cuenta del tiempo :: 0x80
			5 := si es 1, Indica que esta última nota sobrepasó los ticks asignados a la frase ó es la última de esta y debe sonar distintiva
			4 := si es 1, Indica que esta nota tiene una duración de 1.5 veces su valor asignado, si bien esta será reflejada en el byte de tiempo, se debe indicar para el instrumento
			[3 - 0] := estos bits corresponden a los estados que puede adoptar cierta nota en determinado instrumento*/
			timing = 0;
			on = 0.0;
			off = 0.0;
			life = 0.0;
			active = false;
			channel = nullptr;
		}
	} note;
	typedef struct sChannel {
		instrument_base* instrument;
		BYTE* sBeat;//Apuntador a un arreglo de BYTE
	}channel;
	class templateNote {
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

			if (dTimeOn > dTimeOff) { // Note is on
				FTYPE dLifeTime = dTime - dTimeOn;

				if (dLifeTime <= dAttackTime)
					dAmplitude = (dLifeTime / dAttackTime) * dStartAmplitude;

				if (dLifeTime > dAttackTime && dLifeTime <= (dAttackTime + dDecayTime))
					dAmplitude = ((dLifeTime - dAttackTime) / dDecayTime) * (dSustainAmplitude - dStartAmplitude) + dStartAmplitude;

				if (dLifeTime > (dAttackTime + dDecayTime))
					dAmplitude = dSustainAmplitude;
			}
			else { // Note is off

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
			if (dAmplitude <= 0.00001)
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
	class VMMM { //Virtual Machine Music Maker
		public: size_t nBeats, nSubBeats, nCurrentBeat, nTotalBeats;
		public: FTYPE fTempo, fBeatTime, fAccumulate, fLifeTime;
		public: channel vecChannel;
		public: std::vector<note> vecNotes;
		public: bool bRepeat;

		public: VMMM(FTYPE tempo = 120.0f, bool repeat = false, size_t beats = 4, size_t subbeats = 4) {
			nBeats = beats;
			nSubBeats = subbeats;
			fTempo = tempo;
			fBeatTime = 60.0f / fTempo;
			fLifeTime = 0;
			nCurrentBeat = 0;
			nTotalBeats = nSubBeats * nBeats;
			fAccumulate = 0;
			bRepeat = repeat;
		}
		//Renombrarse a Generate(fInitialTime) notes invirtiendo la logica del proceso, tocando cada nota con tiempos on y off listos de salida, entregando solo el vector
		//Donde se debe alistar una función de relleno y activación recíproca

		public: size_t Update(FTYPE fElapsedTime) {//Proceso genera notas de acuerdo al tiempo real, planea ejecutarse una sola vez con los tiempos exactos
			vecNotes.clear();//Se asegura que el vector no tenga notas en la recamara en caso de recalcular el tiempo
			fAccumulate += fElapsedTime;//Se acumula el tiempo transcurrido
			BYTE mayorBeat = 0;
			if ((vecChannel.sBeat[nCurrentBeat] & 0xBF) | (!vecChannel.sBeat[nCurrentBeat] & !0xBF))//En caso de una cadena vacía
				return 0;//Se frena el proceso
			fLifeTime = fBeatTime * ((float)vecChannel.sBeat[2] / 256.0f);//Se confía que nunca se enviará una cadena vacía
			while (fAccumulate >= fLifeTime) {//Si el tiempo es mayor o igual al espacio entre Beats, entonces procede
				do {
					nCurrentBeat += 3;//Avanza 3 beats de acuerdo al sistema SNT
					if (vecChannel.sBeat[nCurrentBeat]&0xBF | !vecChannel.sBeat[nCurrentBeat] & !0xBF)//Verifica si el beat corresponde al final de la linea
						if (bRepeat)  //Si debe seguir un ciclo
							nCurrentBeat = 0;//Se reinica en dado caso, puede simplificarse con una operación de módulo
						else
							return 0;
					if (vecChannel.sBeat[nCurrentBeat] & 0x40) {//Si el estado no es de silencio
						note n;//Debe construirse la nota
						n.channel = vecChannel.instrument;
						n.active = true;
						n.id = vecChannel.sBeat[nCurrentBeat + 1];//De acuerdo a la nota tocada
						n.life = fBeatTime * ((float)vecChannel.sBeat[nCurrentBeat + 2] / 256.0f);
						vecNotes.push_back(n);
					}

					if (vecChannel.sBeat[nCurrentBeat + 2] > mayorBeat)//En caso de encontrarse en un acorde, este tomará el mayor beat
						mayorBeat = vecChannel.sBeat[nCurrentBeat + 2];
					fLifeTime = fBeatTime * ((float)mayorBeat / 256.0f);
					fAccumulate -= fLifeTime;//Se decrementa al tiempo transcurrido, funciona como un enfriamiento
				} while (vecChannel.sBeat[nCurrentBeat] & 0x80);//Repetir las notas que se manden en el mismo tiempo
				mayorBeat = 0;
			}
			return vecNotes.size();
		}

		public: void setInstrument(instrument_base* inst) {
			  channel c;
			  c.instrument = inst;
			  vecChannel = c;
		}

	};
};