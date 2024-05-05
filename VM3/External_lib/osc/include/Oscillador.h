#pragma once
#ifdef OSC_EXPORTS
#define OSC_API __declspec(dllexport)
#else
#define OSC_API __declspec(dllimport)
#endif

#include <cmath>

#ifndef FTYPE
#define FTYPE double
#endif

#ifndef BIT8
#define BIT8 unsigned char
#endif

#ifndef PI
#define PI 2.0 * std::acos(0.0)
#endif

namespace Oscillador {
    extern "C" OSC_API FTYPE scale(size_t half_tone, FTYPE base, size_t id_scale);
    extern "C" OSC_API FTYPE w(FTYPE dHertz);
    extern "C" OSC_API FTYPE sine(FTYPE globalTime, FTYPE ampl, FTYPE dFreq, FTYPE vfq, FTYPE vam, FTYPE org);
    extern "C" OSC_API FTYPE square(FTYPE globalTime, FTYPE ampl, FTYPE dFreq, FTYPE work, FTYPE isPulse, FTYPE org);
    extern "C" OSC_API FTYPE triangle(FTYPE globalTime, FTYPE ampl, FTYPE dFreq, FTYPE vfq, FTYPE vam, FTYPE org);
    extern "C" OSC_API FTYPE saw(FTYPE globalTime, FTYPE ampl, FTYPE dFreq, FTYPE vfq, FTYPE vam, FTYPE det);
    extern "C" OSC_API FTYPE noise(FTYPE globalTime, FTYPE ampl, FTYPE seed, FTYPE begin, FTYPE duration);
    extern "C" OSC_API FTYPE osc(BIT8 tp, FTYPE globalTime, FTYPE ampl, FTYPE dFreq, FTYPE va1, FTYPE va2, FTYPE va3);
    extern "C" OSC_API size_t round_i(FTYPE n);
};
