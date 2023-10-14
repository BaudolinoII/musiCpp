#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>

namespace AdvCon{
	/// <summary>
	/// Clase Singleton para el manejo avanzado de la consola
	/// </summary>
	class AdvanceConsole {
		private: static AdvanceConsole* AC;
		private: HANDLE hConsole; CONSOLE_SCREEN_BUFFER_INFO csbi;
		private: COORD Size;

		public: const char* ESC = "\x1b";
		public: const char* CSI = "\x1b[";

		protected: AdvanceConsole() {
			hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			GetConsoleScreenBufferInfo(this->hConsole, &this->csbi);
        	Size.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
			Size.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		}
		/// <summary>
		/// Suministra el único apuntador de la clase
		/// </summary>
		public: inline static AdvanceConsole* getAC() {
			if (AC == nullptr) AC = new AdvanceConsole();
			return AC;
		}
		/// <summary>
		/// Limpia la consola
		/// </summary>
		/// <param name="x">Coordenada en X(Caracter)</param>
		/// <param name="y">Coordenada en Y(Fila)</param>
		/// <param name="nc">Caracteres a borrar</param>
		public: void clearConsole(const size_t x = 0, const size_t y = 0, const size_t nc = 0) {
			DWORD cCharWritten, dwConSize;

			if (nc > 0)
				dwConSize = static_cast<DWORD>(nc);
			else
				dwConSize = this->csbi.dwSize.X * this->csbi.dwSize.Y;

			if (!FillConsoleOutputCharacter(this->hConsole,(TCHAR)' ',dwConSize,{static_cast<short>(x),static_cast<short>(y)},&cCharWritten))
				return;
			SetConsoleCursorPosition(this->hConsole,{static_cast<short>(x),static_cast<short>(y)});
		}
		/// <summary>
		/// Cambia el cursor de lugar
		/// </summary>
		/// <param name="x">Coordenada en X(Caracter)</param>
		/// <param name="y">Coordenada en Y(Fila)</param>
		public: inline void setXY(const size_t x = 0, const size_t y = 0) {
			SetConsoleCursorPosition(this->hConsole, { static_cast<short>(x), static_cast<short>(y) });
		}
		/// <summary>
		///  Establece el color y la intensidad del texto
		/// </summary>
		/// <param name="textType">
		/// 0.- Texto Rigido no modificable por el usuario Verde intenso
		/// 1.- Texto Generado por el usuario Cyan Intenso
		/// 2.- Texto Del Sistema Blanco por defecto
		/// </param>
		public: void setTextAttributes(const size_t textType) {
			short attributes = 0x0000;
			switch (textType) {
			case 0://Texto Rigido (no se modifica)
				attributes = 0x000A; break;//Verde Intenso
			case 1://Texto Generado Usuario
				attributes = 0x000B; break;//Cyan Intenso
			case 2://Texto Desplegado
				attributes = 0x000F; break;//Blanco
			default:
				attributes = 0x0007;//Gris
			}
			SetConsoleTextAttribute(this->hConsole, attributes);
		}
		/// <summary>
		/// Escucha en paralelo si la tecla dada es presionada
		/// </summary>
		/// <param name="key">numero de tecla a escuchar en Unicode</param>
		/// <returns>true si la tecla es presionada y soltada</returns>
		public: inline bool keyListener(const size_t key) {
			return GetAsyncKeyState(static_cast<int>(key)) & 0x0001;
		}
		/// <summary>
		/// Dibuja un recuadro de color Verde intenso, construido a medida de la consola
		/// </summary>
		public: void drawMainFrame(){
			GetConsoleScreenBufferInfo(this->hConsole, &this->csbi);
			Size.X = csbi.srWindow.Right - csbi.srWindow.Left + 1;
			Size.Y = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
			std::wcout << ESC << "(0"; // Modo Línea
	        std::wcout << CSI << "40;32ml"; // Fondo Negro // Color Verde y linea superior
			for (size_t i = 1; i < Size.X - 1; i++)
	            std::wcout << "q";
			std::wcout << "k";

	        for (size_t i = 1; i < Size.Y - 1; i++) { //Líneas Laterales
	            setXY(0, i);
	            std::wcout << "x";
	            setXY(Size.X - 1, i);
	            std::wcout << "x";
	        }
	        std::wcout << "m";//Línea Inferior
	        for (size_t i = 1; i < Size.X - 1; i++)
	            std::wcout << "q";
	        std::wcout << "j";

	        std::wcout << CSI << "0m";
	        std::wcout << ESC << "(B"; // Restauración de la Consola
	        setXY(1, 1);
		}
		
		/// <summary>
		/// Estructura que contiene y administra las opciones anidadas en el men�
		/// </summary>
		public: typedef struct sTitleNode {
			private: std::string title;
			private: std::vector<std::string> options;
			private: bool mode;
			/// <summary>
			/// Contenedor de opciones, debe iniciarse con un nombre
			/// </summary>
			/// <param name="title">Nombre de la categoria</param>
			/// <param name="mode">
			/// True = modo Explorador.- Controlado por las flechas direccionales, ESC e INTRO
			/// False = modo Captura.- Captura texto usando las propiedades comunes de la consola
			/// </param>
			public: sTitleNode(const std::string& title, const bool mode = false) {
				this->title = title;
				this->mode = mode;
			}
			/// <summary>
			/// Anida una opción para seleccionar, si es que tiene alguna
			/// </summary>
			/// <param name="option">Cadena con la opción a colocar</param>
			public: inline void setOption(const std::string& option) {
				this->options.push_back(option);
			}

			public: inline const bool getMode() {
				return this->mode;
			}
			public: inline void switchMode(){
				this->mode = !this->mode;
			}
			public: inline size_t getOptionSize() {
				return this->options.size();
			}
			public: inline std::string getTitle() {
				return this->title;
			}
			public: inline std::string getOption(const size_t index) {
				if (this->options.empty())
					return "";
				return this->options.at(index % this->options.size());
			}
		}titleNode;
	};
	AdvanceConsole* AdvanceConsole::AC = nullptr;
}