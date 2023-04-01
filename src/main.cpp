#include<iostream>
#include<string>
#include<Windows.h>

//#include "GUI_FVMT.h"
//#include "FileKeeper.h"
#include "CompMusic.h"

//FVMT::FVMT_CONSOLE* console = FVMT::FVMT_CONSOLE::getFVMT();

//void topMenu(){
//	//console->setVisibleCursor(false);
//	while (!console->pressKey(27)) {//ESC
//		console->modeScoutArrows();//Estado
//		console->setMainMenu();//Impresion de Pantalla
//		Sleep(30);
//	}
//}
//void fileTest() {
//	filek::scMachine::setPassword("Stairway_to_Heaven");
//	filek::dynamicfile df, df1, df2; 
//	df["Proyecto"].setString("Ese parametro no se va a guardar");
//	df["Proyecto"]["Nombre"].setString("Mi Opera Prima");
//	df["Proyecto"]["Estilo"].setString("Grand Piano");
//	df["Proyecto"]["Parametros"]["Region"].setString("Occidental");
//	df["Proyecto"]["Parametros"]["Ritmo"].setString("Allegro");
//	df["Proyecto"]["Parametros"]["Clave"].setString("Sol_4");
//
//	filek::dynamicfile::writeFile(df, "test\\info_unc.txt");
//	filek::dynamicfile::writeFile(df, "test\\info_unc.sfc", true);
//}


int main() {
	//fileTest();
	//topMenu();
	std::string example("Do Re#, Mi Fm Solb: LaM, Bb_16+ 7add6; <Do Mi_4,Sol>_32");
	std::string code;
	music::cmpMachine::compile(example, code);
	//console->setInputFeatures(0x000F);
	return 0;
}