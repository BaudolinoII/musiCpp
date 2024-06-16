
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
#include"xml_lib/pugixml.hpp"

#ifndef FTYPE
#define FTYPE double
#endif

#ifndef BIT8
#define BIT8 unsigned char
#endif

#ifndef PI
#define PI 2.0 * std::acos(0.0)
#endif


class Note {
	public: BIT8 id, status, timer;
	public: int scale;
	public: FTYPE on, off;
	public: bool active;
	public: Instrument_xml* channel;
	public: Note();
	
};

class Sound_Model {
	private: FTYPE *temp_val, *val_mb;
	private: std::vector<FTYPE*> ops_val;
	private: std::string note_name;
	private: FTYPE base;
	private: char adj_scale;

	public: Sound_Model(std::string note_name);

	public: void set_temp(std::string temp);
	public: void set_base(std::string base);
	public: void set_adj_scale(std::string adj_scale);
	public: void add_ops(std::string op);
	public: bool identify(std::string note_name);

	public: FTYPE getAmplitude(const FTYPE dTime, const FTYPE dOnTime, const FTYPE dOffTime, const FTYPE dMLT);
	public: FTYPE getSound(const FTYPE dlTime, const FTYPE dMLT, const Note note, bool& bNoteFinished);
};
class Instrument_xml {
	private: std::string name;
	private: FTYPE dMLT = 1.0;
	private: FTYPE dVol = 1.0;
	private: bool isUniversal, isDetailed;

	private: pugi::xml_document doc;
	private: std::vector<Sound_Model> models;

	public: int load_document(const std::string path);
	public: FTYPE getSound(const FTYPE dTime, const Note note, bool& bNoteFinished);
	public: void print_attribs();
	public: std::string getName();
};