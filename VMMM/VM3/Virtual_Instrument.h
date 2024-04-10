#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<cmath>
#include "xml_lib/pugixml.hpp"
#include "Oscillador.h"

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
    public: FTYPE on, off;	 // Tiempo de nota presionada y liberada; Tiempo que permanecer� viva en generacion autom�tica
    public: bool active;			 // Bandera de activacion para ser descartada en el vector principal
    public: Instrument_xml *channel;
      /*Significado de los Bits de Status
        7 := si es 1, El generador de notas debe esperar a otras notas  hasta recibir un estatus de 0 :: 0xF0
        6 := si es 0, Esta nota corresponde a un silencio ignora el tono y solo aumenta la cuenta del tiempo :: 0x80
        5 := si es 1, Indica que esta �ltima nota sobrepas� los ticks asignados a la frase � es la �ltima de esta y debe sonar distintiva
        4 := si es 1, Indica que esta nota tiene una duraci�n de 1.5 veces su valor asignado, si bien esta ser� reflejada en el BIT8 de tiempo, se debe indicar para el instrumento
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


class Sound_Model {
    private: FTYPE* temp_val, *val_mb;
    private: char scale;
    private: std::vector<std::pair<char, FTYPE*>> ops_val;
    private: std::string note_name;

    public: Sound_Model(std::string note_name){
        this->note_name = note_name;
        this->temp_val = new FTYPE[7];
        this->val_mb = new FTYPE[6];
    }

    public: void set_temp(std::string temp) {
        this->temp_val = new FTYPE[7];
        this->val_mb = new FTYPE[6];
        size_t pos = temp.find_first_of(','), begin = 0, i = 0;
        while (pos != std::string::npos) {
            this->temp_val[i] = std::stod(temp.substr(begin, pos - (int)begin));
            begin = pos + 1;
            pos = temp.find_first_of(',', begin);
            i++;
        }
        this->val_mb[0] = (this->temp_val[4] / this->temp_val[0]); //Pendiente Attack
        this->val_mb[1] = (this->temp_val[5] - this->temp_val[4]) / this->temp_val[1];//Pendiente Decay
        this->temp_val[1] += this->temp_val[0];
        this->val_mb[2] = this->temp_val[5] - this->val_mb[1] * (this->temp_val[1]);//Origen Decay
        this->val_mb[3] = (this->temp_val[6] - this->temp_val[5]) / this->temp_val[2];//Pendiente Sustain
        this->temp_val[2] += this->temp_val[1];
        this->val_mb[4] = this->temp_val[6] - this->val_mb[3] * (this->temp_val[2]);//Origen Sustain
        this->val_mb[5] = -this->temp_val[6] / this->temp_val[3];//Pendiente Release
    }
    public: void set_scale(char scale) { this->scale = scale; }
    public: void add_ops(std::string op) {
        FTYPE* vals = new FTYPE[5];
        size_t pos = op.find_first_of(',', 2), begin = 2, i = 0;
        while (pos != std::string::npos) {
            vals[i] = std::stod(op.substr(begin, pos - (int)begin));
            begin = pos + 1;
            pos = op.find_first_of(',', begin);
            i++;
        }
        this->ops_val.push_back({op[0], vals});
        //this->ops_val.back().second[i] = std::stod(op.substr(begin, op.size() - (int)begin));
    }
    public: bool identify(std::string note_name) {
        return !this->note_name.compare(note_name);
    }

    public: FTYPE getAmplitude(const FTYPE dTime, const FTYPE startTime, const FTYPE endTime, const FTYPE dMLT) {
        FTYPE life_time = dTime - startTime;//Tiempo que lleva viva la nota

        if (dTime > endTime && endTime > 0.0)//Se ha alcanzado el final programado
            return (life_time - (this->temp_val[3] + endTime - startTime)) * val_mb[5];

        if (life_time >= (dMLT - this->temp_val[3]))//Esta por sobrepasar el tiempo maximo de vida
            return (life_time - dMLT) * val_mb[5];

        if (life_time < this->temp_val[0])//Attack
            return life_time * val_mb[0];

        if (life_time < this->temp_val[1])//Decay
            return life_time * val_mb[1] + val_mb[2];

        if (life_time < this->temp_val[2])//Sustain
            return life_time * val_mb[3] + val_mb[4];

        return this->temp_val[6];//Keep Sustaining
    }
    public: FTYPE getSound(const FTYPE dTime, const FTYPE dMLT, const Note note, bool& bNoteFinished) {
        FTYPE ampl = getAmplitude(dTime, note.on, note.off, dMLT);
        bNoteFinished = (ampl <= 0.0);//LA nota acabar� si �l m�xmimo de vida es alcanzado y su amplitud es > 0
        FTYPE sound = 0.0, ds = 0.0;
        for (std::pair<char, FTYPE*> op : this->ops_val) {
            size_t id_base = Oscillador::ident_note(op.second[1]);
            ds = Oscillador::scale(id_base) - op.second[1];
            sound += op.second[0] * Oscillador::osc(dTime, Oscillador::scale(id_base + note.id - scale) + ds, op.first, op.second[2], op.second[3], op.second[4]);
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
            std::string note_temp = "";
            pugi::xml_node ampl = note.child("template").child("amplitude");
            for (pugi::xml_attribute attr : ampl.attributes()) {
                note_temp.append(attr.value());
                note_temp.append(",");
            }
            
            Sound_Model sm(note.attribute("from").value());
            sm.set_temp(note_temp);
            pugi::xml_node ops = note.child("method");
            sm.set_scale(std::stoi(ops.attribute("fund").value()));
            std::string note_meth;
            for (pugi::xml_node op = ops.first_child(); op; op = op.next_sibling()) {
               if(!op.attributes().empty())
                   for (pugi::xml_attribute attr : op.attributes()) {//Atributos de la nota
                       note_meth.append(attr.value());
                       note_meth.append(",");
                   }
                
                if (!note_meth.empty()) {
                    sm.add_ops(note_meth);
                }
                note_meth.clear();
            }
            this->models.push_back(sm);
        }
        return 0;
    }
    public: FTYPE getSound(const FTYPE dTime, const Note note, bool& bNoteFinished) {
        if (this->isUniversal) 
            return this->models[0].getSound(dTime, this->dMLT, note, bNoteFinished);
        return models[(note.id % 12)].getSound(dTime, this->dMLT, note, bNoteFinished);
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
    public: std::string getName() {
        return this->name;
    }
};