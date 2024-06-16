/*#ifdef _WIN32
	#ifdef BUILDING_DLL
		#define DLL_EXPORT __declspec(dllexport)
	#else
		#define DLL_EXPORT __declspec(dllimport)
	#endif
#else
	#define DLL_EXPORT
#endif*/

#include<string>
#include<vector>

#ifndef BIT8
#define BIT8 unsigned char
#endif

class MelodyComp {
	public: BIT8 currStatus = 0x40;

	public: const std::vector<std::string> occ_scale12 = { "La", "La#", "Si", "Do", "Do#", "Re", "Re#", "Mi", "Fa", "Fa#", "Sol", "Sol#" };
	public: const std::vector<std::string> chdCms = { "13M","13","11M","11","9M","9","#9","B9","7M","7","/6","6","MAY","MAJ","MIN","MEN","AUM","AUG","DIS","DIM","SUS","5","#5","B5","4","3","2" };
	public: const std::string HEX = "0123456789ABCDEF";
	public: const std::string splitMark = " \t\n\r";
	public: const std::string splitCap = ",;:";
	public: const BIT8 C0 = 0x00, C1 = 0x0C, C2 = 0x18, C3 = 0x24, C4 = 0x30, C5 = 0x3C;
	public: const BIT8 RN = 0x80, BL = 0x40, NG = 0x20, CH = 0x10, SH = 0x08, QH = 0x04, OH = 0x02, DH = 0x01;

	public: bool existElement(const std::string str, const std::string arg);
	public: bool existAnyof(const std::string str, const std::string args);
	public: bool existAllof(const std::string str, const std::string args);
	public: bool existElement(const std::string str, const char arg);

	public: void printArr(BIT8* arr);
	public: void toUpperCase(std::string& str);
	public: void deleteElement(std::string& str, const std::string target);
	public: void deleteChars(std::string& str, const std::string target);
	public: void replaceElements(std::vector<std::string>& vec_str, size_t& index, const std::vector<std::string> vec_tar);
	public: void switchElements(std::vector<std::string>& vec_str, size_t a, size_t b);

	public: int countElement(const std::string str, const char target);
	public: int locateElement(const std::string str, const std::string args, const size_t cap);

	public: size_t near2P(const size_t num);

	public: std::string getUpperCase(const std::string str);

	public: BIT8* VtoA(std::vector<BIT8> vec);
	public: std::vector<std::string> split(const std::string args);
	public: std::vector<std::string> deconst2P(const size_t num);
	public: std::string extractChordTime(std::string& closeMark, BIT8 defaultTime);

	public: bool update_status(std::string arg, BIT8& status);
	public: BIT8 atomize_tone(std::string arg, BIT8 scale);
	public: BIT8 atomize_time(std::string arg, BIT8 bUnit);
	public: void makeChords(std::vector<std::string>& args, size_t& i, bool isNotOnChord);
	public: void unpackChords(std::vector<std::string>& args, size_t nBeats, BIT8 scale, BIT8 bUnit);

	public: BIT8* compileMelody(std::string str, BIT8 scale, size_t nBeats, BIT8 bUnit);
	public: BIT8* compileFile(std::string path);
};