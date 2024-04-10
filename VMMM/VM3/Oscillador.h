#pragma once
#include <string>
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
    static std::string occ_scale[] = { "si", "do","do#","re","re#","mi","fa","fa#","sol","sol#","la","la#" };
    static FTYPE scale(size_t id, size_t id_scale = 0) {
        switch (id_scale) {
        case 0:
            return pow(2.0, (FTYPE)(36 + id) / 12.0);//Frecuencia
        default:
            return 0.0;
        }
    }
    static FTYPE w(FTYPE dHertz) {
        return dHertz * 2.0 * PI;
    }
    static FTYPE osc(FTYPE dt, FTYPE fq, BIT8 tp = 's', FTYPE vfq = 0.0, FTYPE vam = 0.0, const size_t det = 10) {
        FTYPE dFreq = w(fq) * dt + vam * fq * (sin(w(vfq) * dt));// osc(dTime, dLFOHertz, OSC_SINE);
        switch (tp) {
        case 's': // Sine wave bewteen -1 and +1
            return sin(dFreq);
        case 'q': // Square wave between -1 and +1
            return sin(dFreq) > 0 ? 1.0 : -1.0;
        case 't': // Triangle wave between -1 and +1
            return asin(sin(dFreq)) * (2.0 / PI);
        case 'a': {// Saw wave (analogue / warm / slow)
            FTYPE dOutput = 0.0;
            for (size_t n = 1; n < det; n++)
                dOutput += (sin(dFreq * (FTYPE)n)) / (FTYPE)n;
            return dOutput * (2.0 / PI);
        }
        case 'o':
            return (2.0 / PI) * (fq * PI * fmod(dt, 1.0 / fq) - (PI / 2.0));
        case 'n':
            return 2.0 * ((FTYPE)rand() / (FTYPE)RAND_MAX) - 1.0;
        default:
            return 0.0;
        }
    }
    static size_t round_i(FTYPE n) {
        if (std::abs(n) - std::abs(std::floor(n)) < 0.5)
            return std::floor(n);
        return std::ceil(n);
    }
    static size_t ident_note(FTYPE freq) {
        return std::ceil(12.0 * std::log2(freq)) - 36;
    }
}