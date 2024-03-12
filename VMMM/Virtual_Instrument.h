#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<cmath>
#include "xml_lib/pugixml.hpp"

#ifndef FTYPE
#define FTYPE double
#endif

#ifndef BIT8
#define BIT8 unsigned char
#endif

#ifndef PI
#define PI 2.0 * std::acos(0.0)
#endif

class Instrument_xml;
class Note {
    public: BIT8 id, status, timing; // Posicion en la nota y estado de esta y el tiempo representado por Redondas, Blancas, Negras, etc
    public: FTYPE on, off;	 // Tiempo de nota presionada y liberada; Tiempo que permanecerá viva en generacion automática
    public: bool active;			 // Bandera de activacion para ser descartada en el vector principal
    public: Instrument_xml *channel;
      /*Significado de los Bits de Status
        7 := si es 1, El generador de notas debe esperar a otras notas  hasta recibir un estatus de 0 :: 0xF0
        6 := si es 0, Esta nota corresponde a un silencio ignora el tono y solo aumenta la cuenta del tiempo :: 0x80
        5 := si es 1, Indica que esta última nota sobrepasó los ticks asignados a la frase ó es la última de esta y debe sonar distintiva
        4 := si es 1, Indica que esta nota tiene una duración de 1.5 veces su valor asignado, si bien esta será reflejada en el BIT8 de tiempo, se debe indicar para el instrumento
       [3 - 0] := estos bits corresponden a los estados que puede adoptar cierta nota en determinado instrumento*/
    public: Note() {
        this->id = 0;
        this->status = 0;
        this->timing = 0;
        this->on = 0.0;
        this->off = 0.0;
        this->active = false;
        this->channel = nullptr;
    }
 };

namespace Oscillador {
    const std::vector<std::string> occ_scale = { "si", "do","do#","re","re#","mi","fa","fa#","sol","sol#","la","la#" };
    FTYPE scale(size_t id, size_t id_scale = 0) {
        switch (id_scale) {
            case 0:
                return pow(2.0, (FTYPE)(36 + id) / 12.0);//Frecuencia
            default:
                return 0.0;
        }
    }
    FTYPE w(FTYPE dHertz) {
        return dHertz * 2.0 * PI;
    }
    FTYPE osc(FTYPE dt, FTYPE fq, BIT8 tp = 's', FTYPE vfq = 0.0, FTYPE vam = 0.0, const size_t det = 10) {
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
    size_t round_i(FTYPE n) {
        if (std::abs(n) - std::abs(std::floor(n)) < 0.5)
            return std::floor(n);
        return std::ceil(n);
    }
    size_t ident_note(FTYPE freq) {
        return std::ceil(12.0 * std::log2(freq)) - 36;
    }
};
class Sound_Model {
    private: FTYPE* temp_val;
    private: std::vector<std::pair<char, FTYPE*>> ops_val;
    private: std::string note_name;

    public: Sound_Model(std::string note_name){
        this->note_name = note_name;
    }

    public: void set_temp(std::string temp) {
        this->temp_val = new FTYPE[7];
        size_t pos = temp.find_first_of(','), begin = 0, i = 0;
        while (pos != std::string::npos) {
            this->temp_val[i] = std::stod(temp.substr(begin, pos - (int)begin));
            begin = pos + 1;
            pos = temp.find_first_of(',', begin);
            i++;
        }
        //this->temp_val[i] = std::stod(temp.substr(begin, temp.size() - (int)begin));

}
    public: void add_ops(std::string op) {
        this->ops_val.push_back({ op[0],new FTYPE[4] });
        size_t pos = op.find_first_of(',', 2), begin = 2, i = 0;
        while (pos != std::string::npos) {
            this->ops_val.back().second[i] = std::stod(op.substr(begin, pos - (int)begin));
            begin = pos + 1;
            pos = op.find_first_of(',', begin);
            i++;
        }
        //this->ops_val.back().second[i] = std::stod(op.substr(begin, op.size() - (int)begin));
    }

    public: bool identify(std::string note_name) {
        return !this->note_name.compare(note_name);
    }

    public: FTYPE getAmplitude(const FTYPE dTime, const FTYPE startTime, const FTYPE endTime) {
        FTYPE life_time = dTime - startTime;
        FTYPE pend = (this->temp_val[4] / this->temp_val[0]);
        FTYPE org = 0.0;
         
        if (life_time < this->temp_val[0])//Attack
            return life_time * pend + org;
            
        pend = (this->temp_val[5] - this->temp_val[4]) / this->temp_val[1];
        org = this->temp_val[5] - pend * (this->temp_val[0] + this->temp_val[1]);

        if (life_time < (this->temp_val[1] + this->temp_val[0]))//Decay
            return life_time * pend + org;

        pend = (this->temp_val[6] - this->temp_val[5]) / this->temp_val[2];
        org = this->temp_val[6] - pend * (this->temp_val[0] + this->temp_val[1] + this->temp_val[2]);
        if (life_time < (this->temp_val[2] + this->temp_val[1] + this->temp_val[0]))//Sustain
            return life_time * pend + org;
        
        if (endTime == 0.0)
            return this->temp_val[6];
        
        pend = -this->temp_val[6] / this->temp_val[3];
        org = -pend * (this->temp_val[3] + this->temp_val[2] + this->temp_val[1] + this->temp_val[0]);
        FTYPE res = life_time * pend + org;
        if (res > 0.0)
            return res;
        return 0.0;
    }
    public: FTYPE getSound(const FTYPE dTime, const FTYPE dMLT, const Note note, bool& bNoteFinished) {
        FTYPE ampl = getAmplitude(dTime, note.on, note.off);
        bNoteFinished = (dMLT > 0.0 && (dTime - note.on) >= dMLT) || (ampl <= 0.0);
        FTYPE sound = 0.0;
        for (std::pair<char, FTYPE*>op : this->ops_val)
            sound += op.second[0] * Oscillador::osc(dTime, op.second[1], op.first, op.second[2], op.second[3], op.second[4]);
        return sound * ampl;
    }
    public: FTYPE getSound_by_id(const FTYPE dTime, const FTYPE dMLT, const Note note, bool& bNoteFinished) {
        FTYPE ampl = getAmplitude(dTime, note.on, note.off);
        bNoteFinished = (dMLT > 0.0 && (dTime - note.on) >= dMLT) || (ampl <= 0.0);
        FTYPE sound = 0.0;
        for (std::pair<char, FTYPE*>op : this->ops_val) {
            size_t id_base = Oscillador::ident_note(op.second[1]) + (note.id);
            sound += op.second[0] * Oscillador::osc(dTime, Oscillador::scale(id_base), op.first, op.second[2], op.second[3], op.second[4]);
        }   
        return sound * ampl;
    }
};
class Instrument_xml {
    private: std::string name;
    private: FTYPE dMLT = 1.0;
    private: FTYPE dVol = 1.0;
    private: bool isUniversal;

    private: pugi::xml_document doc;
    private: std::vector<Sound_Model> models;

    public: int load_document(const std::string path) {
        if (!this->doc.load_file(path.c_str())) return -1;
        pugi::xml_node notes = this->doc.child("instrument");
        this->name = notes.attribute("name").value();
        this->dMLT = std::stod(notes.attribute("mlt").value());
        this->dVol = std::stod(notes.attribute("vol").value());
        std::string fnote = notes.first_child().attribute("from").value();
        this->isUniversal = !fnote.compare("all");
        for (pugi::xml_node note = notes.first_child(); note; note = note.next_sibling()) {
            std::string note_temp = "", note_meth="";
            pugi::xml_node ampl = note.child("template").child("amplitude");
            for (pugi::xml_attribute attr : ampl.attributes()) {
                note_temp.append(attr.value());
                note_temp.append(",");
            }
            
            Sound_Model sm(note.attribute("from").value());
            sm.set_temp(note_temp);
            pugi::xml_node ops = note.child("method");
            
            for (pugi::xml_node op = ops.first_child(); op; op = op.next_sibling()) {
                note_meth = "";
                for (pugi::xml_attribute attr : op.attributes()) {//Atributos de la nota
                    note_meth.append(attr.value());
                    note_meth.append(",");
                }
                //std::cout << "Checkpoint reached: " << note_meth << "|" << std::endl;
                if(!note_meth.empty())
                    sm.add_ops(note_meth);
                
            }
            
            this->models.push_back(sm);
        }
        return 0;
    }
    public: FTYPE getSound(const FTYPE dTime, const Note note, bool& bNoteFinished) {
        if (this->isUniversal) 
            return this->models[0].getSound_by_id(dTime, this->dMLT, note, bNoteFinished);
        for (Sound_Model model : this->models) {
            if(model.identify(Oscillador::occ_scale[note.id]))
                return model.getSound(dTime, this->dMLT, note, bNoteFinished);
        }
        return 0.0;
    }
    public: void print_attribs() {
        std::cout << "Atributos de instrument: " << std::endl;
        pugi::xml_node notes = this->doc.child("instrument");
        for (pugi::xml_attribute attr : notes.attributes())//Atributos de la clase instrumento
            std::cout << attr.name() << " = " << attr.value() << std::endl;
        for (pugi::xml_node note = notes.first_child(); note; note = note.next_sibling()) {
            std::cout << "\tAtributos de nota: " << std::endl;
            for (pugi::xml_attribute attr : note.attributes())//Atributos de la nota
                std::cout << "\t" << attr.name() << " = " << attr.value() << std::endl;

            pugi::xml_node templ = note.child("template");
            std::cout << "\t\tAtributos de la plantilla: " << std::endl;
            for (pugi::xml_attribute attr : templ.attributes())//Atributos de la plantilla
                std::cout << "\t\t" << attr.name() << " = " << attr.value() << std::endl;
            for (pugi::xml_node ampl = templ.first_child(); ampl; ampl = ampl.next_sibling()) {
                std::cout << "\t\t\tAtributos de la Amplitud: " << std::endl;
                for (pugi::xml_attribute attr : ampl.attributes())//Atributos de la Amplitud
                    std::cout << "\t\t\t" << attr.name() << " = " << attr.value() << std::endl;
            }

            pugi::xml_node meth = note.child("method");
            std::cout << "\t\tMetodo: " << std::endl;
            for (pugi::xml_node op = meth.first_child(); op; op = op.next_sibling()) {
                std::cout << "\t\t\tAtributos de la Operacion: " << std::endl;
                for (pugi::xml_attribute attr : op.attributes())//Atributos de la Operacion
                    std::cout << "\t\t\t" << attr.name() << " = " << attr.value() << std::endl;
            }
        }
    }
};