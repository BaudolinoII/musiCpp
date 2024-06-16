/*#ifdef _WIN32
	#ifdef BUILDING_DLL
		#define DLL_EXPORT __declspec(dllexport)
	#else
		#define DLL_EXPORT __declspec(dllimport)
	#endif
#else
	#define DLL_EXPORT
#endif*/

#ifndef FTYPE
#define FTYPE double
#endif

#ifndef PI
#define PI 2.0 * std::acos(0.0)
#endif

#ifndef BIT8
#define BIT8 unsigned char
#endif

//extern "C" DLL_EXPORT

namespace Oscillador {
	FTYPE w(const FTYPE dHertz);
	FTYPE scale(const int nNoteID, const FTYPE dBase);
	FTYPE osc(const BIT8 nType, const FTYPE dTime, const FTYPE dHertz, FTYPE ampl, FTYPE vfq, FTYPE vam, FTYPE org);
	FTYPE sine(const FTYPE dTime, const FTYPE dHertz, FTYPE ampl, FTYPE vfq, FTYPE vam, FTYPE org);
	FTYPE square(const FTYPE dTime, const FTYPE dHertz, FTYPE ampl, FTYPE work, FTYPE isPulse, FTYPE org);
	FTYPE triangle(const FTYPE dTime, const FTYPE dHertz, FTYPE ampl, FTYPE vfq, FTYPE vam, FTYPE org);
	FTYPE saw(const FTYPE dTime, const FTYPE dHertz, FTYPE ampl, FTYPE vfq, FTYPE vam, FTYPE org);
	FTYPE noise(const FTYPE dTime, FTYPE ampl, FTYPE seed, FTYPE begin, FTYPE duration);
	size_t round_i(FTYPE n);
};