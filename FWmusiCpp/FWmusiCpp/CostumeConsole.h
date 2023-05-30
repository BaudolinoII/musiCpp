#pragma once
#include <iostream>
#include <Windows.h>

namespace AdvCon{
	/// <summary>
	/// Clase Singleton para el manejo avanzado de la consola
	/// </summary>
	class AdvanceConsole {
	private: static AdvanceConsole* AC;
	private: HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	protected: AdvanceConsole() {}
	/// <summary>
	/// Suministra el único apuntador de la clase
	/// </summary>
	public: static AdvanceConsole* getAC() {
		if (AC == nullptr)
			AC = new AdvanceConsole();
		return AC;
	}
	/// <summary>
	/// Limpia la consola
	/// </summary>
	/// <param name="x">Coordenada en X(Caracter)</param>
	/// <param name="y">Coordenada en Y(Fila)</param>
	/// <param name="nc">Caracteres a borrar</param>
	public: void clearConsole(const size_t x = 0, const size_t y = 0, const size_t nc = 0) {
		COORD chrScreen = { static_cast<short>(x), static_cast<short>(y) };
		DWORD cCharWritten, dwConSize;
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!GetConsoleScreenBufferInfo(this->hConsole, &csbi))
			return;
		if (nc > 0)
			dwConSize = nc;
		else
			dwConSize = csbi.dwSize.X * csbi.dwSize.Y;
		if (!FillConsoleOutputCharacter(this->hConsole, (TCHAR)' ', dwConSize, chrScreen, &cCharWritten))
			return;
		SetConsoleCursorPosition(this->hConsole, chrScreen);
	}
	/// <summary>
	/// Cambia el cursor de lugar
	/// </summary>
	/// <param name="x">Coordenada en X(Caracter)</param>
	/// <param name="y">Coordenada en Y(Fila)</param>
	public: void setXY(const size_t x = 0, const size_t y = 0) {
		COORD chr = { static_cast<short>(x), static_cast<short>(y) };
		SetConsoleCursorPosition(this->hConsole, chr);
	}
	/// <summary>
	/// Coloca atributos del siguiente texto a imprimir.
	/// Ahorra el darle un HandleConsole
	/// </summary>
	/// <param name="attributes">Atributos en Bytes</param>
	void setTextAttributes(const short attributes) {
		SetConsoleTextAttribute(this->hConsole, attributes);
	}
	/// <summary>
	/// Escucha en paralelo si la tecla dada es presionada
	/// </summary>
	/// <param name="key">numero de tecla a escuchar en Unicode</param>
	/// <returns>true si la tecla es presionada y soltada</returns>
	bool keyListener(const size_t key) {
		return GetAsyncKeyState(key) & 0x0001;
	}
	};
	AdvanceConsole* AdvanceConsole::AC = nullptr;
}