#include<string>
#include<iostream>
#include<thread>
#include<fstream>
#include<vector>
#include<unordered_map>

#include<Windows.h>

#include "FileKeeper.h"

namespace FVMT {
	class advanceConsole {
		private: size_t WIDTH = 120, HEIGHT = 30;
		private: const COORD ORIGIN = { 0,0 };

		private: HANDLE hConsole; HWND hWC; HDC hCC;
		private: CONSOLE_CURSOR_INFO cci;

		protected:advanceConsole() {
			//General Assight
			this->hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			this->hWC = GetConsoleWindow();
			this->hCC = GetDC(this->hWC);

			this->setVisibleCursor(true);
		}

		public: inline void setVisibleCursor(const bool isActive) {
			this->cci.bVisible = isActive;
			SetConsoleCursorInfo(this->hConsole, &this->cci);
		}
		public: inline void setInputFeatures(const short settings) {
			SetConsoleTextAttribute(this->hConsole, settings);
		}
		public: inline void gotoXY(const short x, const short y) {
			SetConsoleCursorPosition(this->hConsole, { x, y });
		}
		public: inline void gotoXY(const COORD cd) {
			SetConsoleCursorPosition(this->hConsole, cd);
		}

		public: inline const bool pressKey(const short key) {
			return GetAsyncKeyState(key) & 0x0001;
		}

		public: inline void drawLine(const size_t x, const size_t y, const size_t h, const size_t k) {
			MoveToEx(this->hCC, x, y, NULL);
			LineTo(this->hCC, h, k);
			SelectObject(this->hCC, (HPEN)SelectObject(this->hCC, (HPEN)GetStockObject(WHITE_PEN)));
			ReleaseDC(this->hWC, this->hCC);
		}
		public: inline void clearConsole() {
			DWORD cCharsWritten, dwConSize;
			CONSOLE_SCREEN_BUFFER_INFO csbi;
			if (!GetConsoleScreenBufferInfo(this->hConsole, &csbi))
				return;
			dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
			if (!FillConsoleOutputCharacter(this->hConsole, (TCHAR)' ', dwConSize, this->ORIGIN, &cCharsWritten))
				return;
			gotoXY(this->ORIGIN);
		}

		public: void drawMainFrame() {
			this->clearConsole();
			this->gotoXY(0, 0); std::wcout << std::wstring(1, 201); for (unsigned int i = 0; i < (this->WIDTH - 2); i++) std::wcout << std::wstring(1, 205); std::wcout << std::wstring(1, 187);;
			for (unsigned int y = 1; y <= (this->HEIGHT - 2); y++) {this->gotoXY(0, y); std::wcout << std::wstring(1, 186); this->gotoXY(this->WIDTH - 1, y); std::wcout << std::wstring(1, 186);}
			this->gotoXY(0, this->HEIGHT - 1); std::wcout << std::wstring(1, 200); for (unsigned int i = 0; i < (this->WIDTH - 2); i++) std::wcout << std::wstring(1, 205); std::wcout << std::wstring(1, 188);
		}
		public: typedef struct sTitleNode {
			  private: std::string title;
			  private: std::vector<std::string> options;
			  private: bool mode;

			  public: struct sTitleNode(const std::string& title, const bool mode = false) {
				  this->title = title;
				  this->mode = mode;
			  }
			  public: inline void setOption(const std::string& option) {
				  this->options.push_back(option);
			  }

			  public: inline const bool getMode() {
				  return this->mode;
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
			  public: inline COORD getPointCoord(const size_t index, const size_t x0, const size_t y0) {
				  COORD cd = { x0 + this->title.size() + getOption(index).size(), y0 };
				  return cd;
			  }
		}titleNode;
	};
	class FVMT_CONSOLE : public advanceConsole {
		private: static FVMT_CONSOLE* fvmtC;
		private: std::vector<titleNode> menuTitles;
		private: std::string buffer; size_t i = 0; int index = 0, selection = 0;
		private: FVMT_CONSOLE() {
			menuTitles.push_back(titleNode("Crear Nuevo Proyecto........"));
			menuTitles.push_back(titleNode("Editar Proyecto Existente..."));
			menuTitles.push_back(titleNode("Ajustar Afinadores.........."));
			menuTitles.push_back(titleNode("Instrumento Virtual........."));
			menuTitles.push_back(titleNode("Salir......................."));
			menuTitles.at(0).setOption("Vacio");
			menuTitles.at(0).setOption("Estandar");
			menuTitles.at(1).setOption("Mi opera Prima");
			menuTitles.at(1).setOption("Todos Somos Uno");
			menuTitles.at(2).setOption("Piano");
			menuTitles.at(2).setOption("Guitarra");
			menuTitles.at(3).setOption("Piano");
			menuTitles.at(3).setOption("Guitarra");
			menuTitles.at(4).setOption("[ESC]");
			menuTitles.at(4).setOption("[INTRO]");
		}

		public: const size_t modeScoutArrows() {
			if (this->pressKey(VK_UP)) {//UP
				if (selection > 0)
					selection--;
				else
					selection = menuTitles.size() - 1;
				index = 0;
				Sleep(100);
				return 0;
			}
			if (this->pressKey(VK_DOWN)) {//DOWN
				if (selection < menuTitles.size() - 1)
					selection++;
				else
					selection = 0;
				index = 0;
				Sleep(100);
				return 0;
			}
			if (this->pressKey(VK_RIGHT)) {//RIGHT
				if (index < menuTitles.at(selection).getOptionSize() - 1)
					index++;
				else
					index = 0;
				Sleep(100);
				return 0;
			}
			if (this->pressKey(VK_LEFT)) {//LEFT
				if (index > 0)
					index--;
				else
					index = menuTitles.at(selection).getOptionSize() - 1;
				Sleep(100);
				return 0;
			}
			if (this->pressKey(VK_RETURN)) {//ENTER
				return 1;
			}
			return 0;
		}
		public: void setMainMenu() {
			this->clearConsole();
			this->setInputFeatures(0x000F);
			this->drawMainFrame();
			this->setInputFeatures(0x000A);
			this->gotoXY(44, 1); std::cout << "Free Virtual Music Tool V0.2.15";
			this->gotoXY(46, 2); std::cout << "By "; 
			this->setInputFeatures(0x000B); std::cout << "JoGEHrt";
			this->setInputFeatures(0x0002); std::cout << " from "; 
			this->setInputFeatures(0x000E); std::cout << "DynamicWare";
			this->setInputFeatures(0x0002);
			for (titleNode mt : menuTitles) {
				this->gotoXY(42, 5 + i * 2);
				if (selection == i)
					this->setInputFeatures(0x4002);
				else
					this->setInputFeatures(0x0002);
				std::cout << mt.getTitle();
				if (selection == i)
					std::cout << " " << mt.getOption(index);
				else {
					this->setInputFeatures(0x0002);
					std::cout << "                            ";
					this->gotoXY(42 + mt.getTitle().size(), 5 + selection * 2);
				}
				i++;
			}
			i = 0;
		}
		public: const size_t setNameScreen(std::string& buffer,const bool mode = false, const size_t type = 0){
			this->clearConsole();
			if (mode) {
				this->setInputFeatures(FOREGROUND_GREEN); this->gotoXY(41, 5); std::cout << "Como se llamara tu nueva composicion:";
				this->setVisibleCursor(true);
				this->setInputFeatures(FOREGROUND_GREEN | COMMON_LVB_REVERSE_VIDEO); this->gotoXY(41, 7); std::getline(std::cin, buffer);
				this->setVisibleCursor(false);
				return buffer.empty();
			} else {
				this->setInputFeatures(FOREGROUND_GREEN); this->gotoXY(45, 5); std::cout << "Como se llama tu composicion:";
				this->setVisibleCursor(true);
				this->setInputFeatures(FOREGROUND_GREEN | COMMON_LVB_REVERSE_VIDEO); this->gotoXY(45, 7); std::getline(std::cin, buffer);
				this->setVisibleCursor(false);
			}
			return 0;
		}
		public: void setProyectEdit(std::string& name, const size_t tem = 0){}
		public: void setProyectEdit(std::string& name, std::string& param){}
		public: static FVMT_CONSOLE* getFVMT() {
			if (fvmtC == nullptr)
				fvmtC = new FVMT_CONSOLE();
			return fvmtC;
		}
	};
	FVMT_CONSOLE* FVMT_CONSOLE::fvmtC = nullptr;
}