#pragma once
#include <cmath>
#include "Oscillador.hpp"

namespace Oscillador {
    FTYPE w(FTYPE dHertz) {
        return dHertz * 2.0 * PI;
    }
    FTYPE scale(int half_tone, FTYPE base) {
        return base * pow(1.0594630943592952645618252949463, half_tone);
    }
    
    FTYPE sine(FTYPE dTime,  FTYPE dFreq, FTYPE ampl, FTYPE vfq, FTYPE vam, FTYPE org) {
        return ampl * std::sin(w(dFreq) * dTime + vam * dFreq * (std::sin(w(vfq) * dTime))) + org;
    }
    FTYPE square(FTYPE dTime, FTYPE dFreq, FTYPE ampl, FTYPE work, FTYPE isPulse, FTYPE org) {
        return (std::fmod(dTime * dFreq, 1.0) > work ? ampl : (isPulse ? 0.0 : -ampl)) + org;
    }
    FTYPE triangle(FTYPE dTime, FTYPE dFreq, FTYPE ampl, FTYPE vfq, FTYPE vam, FTYPE org) {
        return ampl * std::asin(std::sin(w(dFreq) * dTime + vam * dFreq * (std::sin(w(vfq) * dTime)))) * (2.0 / PI) + org;
    }
    FTYPE saw(FTYPE dTime, FTYPE dFreq, FTYPE ampl, FTYPE vfq, FTYPE vam, FTYPE det) {
        if (det == 0)
            return ampl * (2.0 / PI) * (dFreq * PI * std::fmod(dTime, 1.0 / dFreq) - (PI / 2.0));
        FTYPE dOutput = 0.0, varF = w(dFreq) * dTime + vam * dFreq * (std::sin(w(vfq) * dTime));
        for (FTYPE n = 1.0; n < det; n += 1.0)
            dOutput += (std::sin(varF * n)) / n;
        return ampl * dOutput * (2.0 / PI);

    }
    FTYPE noise(FTYPE dTime, FTYPE ampl, FTYPE seed, FTYPE begin, FTYPE duration) {
        if (begin >= dTime)
            return 0;
        if (duration < 0.0 && (duration + begin) >= dTime) {
            std::srand((size_t)seed);
            return 2.0 * ampl * ((FTYPE)std::rand() / (FTYPE)RAND_MAX) - ampl;
        }
        return 0;
    }

    FTYPE osc(BIT8 tp, FTYPE dTime, FTYPE ampl, FTYPE dFreq, FTYPE va1, FTYPE va2, FTYPE va3) {
        switch (tp) {
        case 0:
            return sine(dTime, ampl, dFreq, va1, va2, va3);
        case 1:
            return square(dTime, ampl, dFreq, va1, va2, va3);
        case 2:
            return triangle(dTime, ampl, dFreq, va1, va2, va3);
        case 3:
            return saw(dTime, ampl, dFreq, va1, va2, va3);
        case 4:
            return noise(dTime, ampl, dFreq, va1, va2);
        default:
            return 0.0;
        }
    }

    size_t round_i(FTYPE n) {
        if (std::abs(n) - std::abs(std::floor(n)) < 0.5)
            return std::floor(n);
        return std::ceil(n);
    }
}