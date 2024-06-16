#include <iostream>
#include <vector>
#include <string>
#include <atomic>
#include <list>

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

		protected: typedef bool(*lambda)(Note const& item);
		protected: template<class T> static void safe_remove(T& v, lambda f);

		protected: static FTYPE MakeNoise(int nChannel, FTYPE globalTime);
		protected: olcNoiseMaker<short> sound;

		protected: void setTime();
		protected: void updateTime();

		public: VMMM();
	};
	std::vector<Note> VMMM::vecNotes;
	std::mutex VMMM::muxNotes;
	template<class T>
	void VMMM::safe_remove(T& t, lambda f) {
		auto n = v.begin();
		while (n != v.end())
			if (f(*n))//Nota activa
				++n;//La deja pasar
			else//Nota inactiva
				n = v.erase(n);//Es eliminada
	}

	class ConsoleSynth : public VMMM {
		private: bool ctl_right = true, ctl_left = true;
		private: char scale = 4;
		private: void print_board();

		public: ConsoleSynth();
		public: void KeyBoard_MainLoop(Instrument_xml* inst);
	};

	typedef struct s_Track {
		public: Instrument_xml* inst;
		public: BIT8* melody;

		public: size_t currBeat, repeat, times;
		public: FTYPE tempo, accm_time, start, lifeTime;

		public: s_Track(Instrument_xml* inst, BIT8* melody, FTYPE tempo, FTYPE start, size_t repeat);
		public: bool isActive();
		public: BIT8 getStatus();
		public: BIT8 getNote();
		public: BIT8 getTime();
		public: FTYPE getTimeSec();
		public: FTYPE getBeforeTimeSec();
		public: void nextBeat();
	}Track;

	class VirtualOrquesta : public VMMM {
		private: std::vector<Track> tracks;
		private: std::vector<Note> curr_notes;

		public: VirtualOrquesta();

		public: void setTrack(Instrument_xml* inst, BIT8* melody, FTYPE tempo, FTYPE start, size_t repeat);
		public: void printInstTracks();
		public: void popTrack(size_t index);

		public: size_t play(FTYPE dTime);
		public: void Concert_MainLoop();
	};
};
