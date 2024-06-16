#include<iostream>
#include<string>
#include<vector>
#include<cmath>

#include "xml_lib\pugixml.hpp"
#include "VirtualInstrument.hpp"
#include "Oscillador.hpp"

Note::Note() {
    this->status = 0;
    this->id = 0;
    this->timer = 0;
    this->scale = 0;
    
    this->on = 0.0;
    this->off = 0.0;
    this->active = false;
}

Sound_Model::Sound_Model(std::string note_name) {
    this->note_name = note_name;
    this->temp_val = new FTYPE[7];
    this->val_mb = new FTYPE[6];
}

void Sound_Model::set_temp(std::string temp) {
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
void Sound_Model::set_base(std::string base){
    this->base = std::stod(base);
}
void Sound_Model::set_adj_scale(std::string adj_scale){
    this->adj_scale = std::stoi(adj_scale);
}
void Sound_Model::add_ops(std::string op) {
    FTYPE* op_val = new FTYPE[4];
    size_t pos = op.find_first_of(',', 2), begin = 2, i = 0;
    while (pos != std::string::npos) {
        op_val[i] = std::stod(op.substr(begin, pos - (int)begin));
        begin = pos + 1;
        pos = op.find_first_of(',', begin);
        i++;
    }
    this->ops_val.push_back(op_val);
}
bool Sound_Model::identify(std::string note_name) {
    return !this->note_name.compare(note_name);
}

FTYPE Sound_Model::getAmplitude(const FTYPE dTime, const FTYPE dOnTime, const FTYPE dOffTime, const FTYPE dMLT) {
    FTYPE life_time = dTime - dOnTime;//Tiempo que lleva viva la nota

    if (dTime > dOffTime && dOffTime > 0.0)//Se ha alcanzado el final programado
        return (life_time - (this->temp_val[3] + dOffTime - dOnTime)) * val_mb[5];

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
FTYPE Sound_Model::getSound(const FTYPE dTime, const FTYPE dMLT, const Note note, bool& bNoteFinished) {
    FTYPE ampl = getAmplitude(dTime, note.on, note.off, dMLT);
    bNoteFinished = (ampl <= 0.0);
    FTYPE sound = 0.0, ds = 0.0;

    const int scaled_note = (int)note.id + 12 * (note.scale - this->adj_scale);
    for (FTYPE* ops : this->ops_val)
        sound += Oscillador::osc(ops[0],dTime,Oscillador::scale(scaled_note + (int)ops[1], this->base),ops[2],ops[3],ops[4],ops[5]);
    return sound * ampl;
}

int Instrument_xml::load_document(const std::string path) {
    std::cout << "Loading File at: " << path << std::endl;
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

        sm.set_base(ops.attribute("base").value());
        sm.set_adj_scale(ops.attribute("adj_scale").value());
        std::string note_meth;

        for (pugi::xml_node op = ops.first_child(); op; op = op.next_sibling()) {
            if (!op.attributes().empty())
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
    std::cout << "Instrument " << this->name << "Succesfully loaded" << std::endl;
    return 0;
}
FTYPE Instrument_xml::getSound(const FTYPE dTime, const Note note, bool& bNoteFinished) {
    if (this->isUniversal)
        return this->models[0].getSound(dTime, this->dMLT, note, bNoteFinished);
    return models[(note.id % 12)].getSound(dTime, this->dMLT, note, bNoteFinished);
}
void Instrument_xml::print_attribs() {
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
std::string Instrument_xml::getName() {
    return this->name;
}