#include<iostream>
#include<functional>
#include<stack>
#include<string>

//musiC++

namespace music {
	class cmpMachine {
		/// <summary>
		/// Procedimiento de compilación del lenguaje musiC++
		/// Esta funcion reciba un argumento para ser procesado y devolvera una cadena de atomos para su posterior interpretacion
		/// </summary>
		/// <param name="args"> :Cadena otorgada por el usuario</param>
		/// <param name="compiled"> :Cadena para lamacenar el lenguaje optimizado para su interpretación</param>
		public: static void compile(std::string args, std::string& compiled) {
			std::string atomicString; char tone;
			size_t axis = args.find_first_of(" \t,;:\0");//Buscará la posición más cercana de los símbolos que pueden dividir un argumento
			int index = 0; std::stack<std::string> pile;

			std::function<void(std::string)> atomize = [&](std::string comm) {
				if (comm.empty())//En caso de estar vacío el comando, se descarta
					return;//Asi podemos depurar la cadema con multiples espacios o redundantes
				std::cout << comm << std::endl;
				//Debemos de obtener el parámetro de tono
				if (comm.size() > 3) {//Un caso excepcional, seria un Sol
					if (comm.substr(0, 3) == "Sol" | (comm[0] == 'G'))	   tone = 'L';
					else if (comm.substr(0, 2) == "Do" | (comm[0] == 'C')) tone = 'A';
					else if (comm.substr(0, 2) == "Re" | (comm[0] == 'D')) tone = 'C';
					else if (comm.substr(0, 2) == "Mi" | (comm[0] == 'E')) tone = 'E';
					else if (comm.substr(0, 2) == "Fa" | (comm[0] == 'F')) tone = 'F';
					else if (comm.substr(0, 2) == "La" | (comm[0] == 'A')) tone = 'H';
					else if (comm.substr(0, 2) == "Si" | (comm[0] == 'B')) tone = 'J';
					else if (comm[0] == 's')tone = ' ';
				}
			};
			//Paso 1: separar los argumentos de la cadena
			while(axis != std::string::npos){
				if((args[index] == ' ' )| (args[index] == '\t' )| (args[index] == '\0'))//Si este argumento es de tipo espacio, tabulador o nulo
					atomize(args.substr(index, axis - index));//se ingresa a la cadena sin este último
				else
					atomize(args.substr(index, axis - index + 1));//Los otros símbolos son necesarios de acuerdo a su contexto
				index = axis + 1;//Se actualiza el index para su siguiente recorte
				axis = args.find_first_of(" ,;:\0", axis + 1);//Busca el próximo argumento
			}
			atomize(args.substr(index, axis - index));
		}
		public: static void settings() {

		}
	};
}