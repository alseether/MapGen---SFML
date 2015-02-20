#include <iostream>
#include <iomanip>
#include <math.h>
#include <Windows.h>
#include <windows.system.h>
#include "Map.hpp"
#include "Conversor.hpp"
#include "Ventana.hpp"

using namespace std;

int main(){
	Map m = Map(9);
	system("cls");
	cout << m.getSeed();
	m.generate(0.7f);	
	Conversor c(m);
	Ventana v("Vistas");
	v.show(c.getVistaPlanta(2));
	v.show(c.getCorte3DLR(2));
	v.show(c.getCorte3DRL(2));
	v.show(c.getCorte3DLRDotted(2));
	v.show(c.getCorte3DRLDotted(2));
	v.show(c.getCorte3DFront(2));
	v.show(c.getCorte3DFrontDotted(2));

	v.close();
	return 0;
}
