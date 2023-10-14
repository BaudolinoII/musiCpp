#pragma once

#ifndef FTYPE
#define FTYPE double
#endif

#include "SoundTools.h"

namespace inst {
	class instrument_bell : public stl::instrument_base {
		public: instrument_bell() {
			tmp.dAttackTime = 0.01;
			tmp.dDecayTime = 1.0;
			tmp.dSustainAmplitude = 0.0;
			tmp.dReleaseTime = 1.0;

			dVolume = 1.0;
		}
		public: FTYPE sound(const FTYPE dTime, stl::note n, bool& bNoteFinished) {
			FTYPE dAmplitude = tmp.GetAmplitude(dTime, n.on, n.off);
			if (dAmplitude <= 0.0) bNoteFinished = true;

			FTYPE dSound =
				+ 1.00 * stl::osc(n.on - dTime, stl::scale(n.id + 12), stl::OSC_SINE, 5.0, 0.001)
				+ 0.50 * stl::osc(n.on - dTime, stl::scale(n.id + 24))
				+ 0.25 * stl::osc(n.on - dTime, stl::scale(n.id + 36));

			return dAmplitude * dSound * dVolume;
		}
	};
	class instrument_bell8 : public stl::instrument_base {
		public: instrument_bell8() {
			tmp.dAttackTime = 0.01;
			tmp.dDecayTime = 0.5;
			tmp.dSustainAmplitude = 0.8;
			tmp.dReleaseTime = 1.0;

			dVolume = 1.0;
		}
		public: FTYPE sound(const FTYPE dTime, stl::note n, bool& bNoteFinished) {
			FTYPE dAmplitude = tmp.GetAmplitude(dTime, n.on, n.off);
			if (dAmplitude <= 0.0) bNoteFinished = true;

			FTYPE dSound =
				+ 1.00 * stl::osc(n.on - dTime, stl::scale(n.id), stl::OSC_SQUARE, 5.0, 0.001)
				+ 0.50 * stl::osc(n.on - dTime, stl::scale(n.id + 12))
				+ 0.25 * stl::osc(n.on - dTime, stl::scale(n.id + 24));

			return dAmplitude * dSound * dVolume;
		}
	};
	class instrument_harmonica : public stl::instrument_base {
		public: instrument_harmonica() {
			tmp.dAttackTime = 0.05;
			tmp.dDecayTime = 1.0;
			tmp.dSustainAmplitude = 0.95;
			tmp.dReleaseTime = 0.1;

			dVolume = 1.0;
		}
		public: FTYPE sound(const FTYPE dTime, stl::note n, bool& bNoteFinished) {
			FTYPE dAmplitude = tmp.GetAmplitude(dTime, n.on, n.off);
			if (dAmplitude <= 0.0) bNoteFinished = true;

			FTYPE dSound =
				//+ 1.0  * stl::osc(n.on - dTime, stl::scale(n.id-12), stl::OSC_SAW_ANA, 5.0, 0.001, 100)
				+ 1.00 * stl::osc(n.on - dTime, stl::scale(n.id), stl::OSC_SQUARE, 5.0, 0.001)
				+ 0.50 * stl::osc(n.on - dTime, stl::scale(n.id + 12), stl::OSC_SQUARE)
				+ 0.05 * stl::osc(n.on - dTime, stl::scale(n.id + 24), stl::OSC_NOISE);

			return dAmplitude * dSound * dVolume;
		}
	};
};