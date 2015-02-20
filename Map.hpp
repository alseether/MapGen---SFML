#ifndef MAP_HPP
#define MAP_HPP
#include <iostream>
#include <iomanip>
#include <set>
#include <math.h>
#include <time.h>
#include <Windows.h>

class Map {
private:

	// ATRIBUTOS DE LA LOGICA DEL MAPA

	/*
	* roughness indica como de arisco es el terreno generado.
	* Los valores normales estan entre 0 y 1, aunque puede tomar valores mayores de 1, creando mapas muy irregulares
	* Cuanto mayor es el valor de roughness, mas variacion puede haber entre dos pixeles adyacentes
	*/
	float roughness;
	
	/*
	* size indica el largo del lado de la matriz (cuadrada) de valores de altura del terreno
	* size se calcula a partir de un nivel de detalle, de tal forma, size siempre es un valor del tipo:
	*	(2^detalle) + 1
	* O lo que es lo mismo, una potencia de 2 mas uno (5,9,17,33,...)
	*/
	int size;
	
	/*
	* max es size-1. Dentro de la implementación, es útil, ya que indica el ultimo valor valido para acceder a la matriz.
	* La matriz se declara de tamaño size x size, con posiciones validas desde 0 hasta size-1.
	* Esta variable facilita la comprension del codigo y la implementacion (Evita poner size-1 en todos lados)
	*/
	int max;
	
	
	
	/*
	* guarda la seed con la que se ha generado el mapa
	*/
	int seed;
	

	// METODOS PRIVADOS

	/**
	* Obtiene el valor de la posicion (x,y) del mapa. Devuelve -1 si la posicion es invalida y el valor en caso contrario
	*/
	float get(int x, int y){
		if (x < 0 || x > this->max || y < 0 || y > this->max) return -1;
		return this->map[x + this->size * y];
	}

	/**
	* Establece el valor de la posicion (x,y) del mapa a val.  No hace nada si la posicion es invalida
	*/
	void set(int x, int y, float val){
		if (x < 0 || x > this->max || y < 0 || y > this->max) return;
		this->map[x + this->size * y] = val;
	}

	/**
	* Calcula la media de 4 valores float (usado en diamantes y cuadrados).
	* Si algun elemento del conjunto es -1 (por intentar hacer media con una posicion fuera de rango), este valor se descarta
	* y se hace la media de el resto de valores validos.
	* Devuelve la media
	*/
	float average(float* values) {
		float suma = 0;
		int elementos = 0;
		for (int i = 0; i < 3; ++i){
			if (values[i] != -1){
				suma += values[i];
				++elementos;
			}
		}
		return suma / elementos;
	}

	/**
	* Realiza la media (cuadrado) de una posicion (x,y) mas un offset dado
	*/
	void square(int x, int y, int size, float offset) {
		float valores[4];
		valores[0] = this->get(x - size, y - size);	  // upper left
		valores[1] = this->get(x + size, y - size);   // upper right
		valores[2] = this->get(x + size, y + size);	  // lower right
		valores[3] = this->get(x - size, y + size);	  // lower left
		float ave = average(valores);
		this->set(x, y, ave + offset);
	}

	/**
	* Realiza la media (diamante) de una posicion (x,y) mas un offset dado
	*/
	void diamond(int x, int y, int size, float offset) {
		float valores[4];
		valores[0] = this->get(x, y - size);	// top
		valores[1] = this->get(x + size, y);	// right
		valores[2] = this->get(x, y + size);	// bottom
		valores[3] = this->get(x - size, y);	// left
		float ave = average(valores);
		this->set(x, y, ave + offset);
	}

	/**
	* Rellena el mapa con los valores de altura, mediante el algoritmo Diamond-Square, de forma recursiva.
	* Actua sobre TODOS los sectores cuadrados del mapa, de lado size. No confundir con this->size,
	* aqui size cada vez es dos veces mas pequeño, actuando primero sobre un cuadrado de tamaño
	* size x size, luego size/2 x size/2, y asi recursivamente, hasta que el lado es 2, donde no se puede realizar
	* ningun calculo mas
	*/
	void divide(int size) {
		int x, y, half = size / 2;
		float scale = this->roughness * size;
		/*
		* scale tiene la funcion de darle "menos peso" a roughness cuanto mas pequeña es la seccion a tratar.
		* Esto evita que haya grandes diferecias de altura en casillas adyacentes, aun poniendo un roughness
		* alto
		*/
		if (half < 1) return;	// CASO BASE, cuando se tratan secciones de 2x2

		for (y = half; y < this->max; y += size) {
			for (x = half; x < this->max; x += size) {
				float r = ((float)rand() / (RAND_MAX));
				square(x, y, half, r * scale * 2 - scale);
			}
		}
		/*
		* Primero se calculan TODAS las medias (tipo square) para todas las subdivisiones de tamaño size x size del mapa
		* Estas medias establecen valores que son necesarios para calcular la medias tipo diamond
		* No voy a explicar la forma de avance de los bucles for, porque no es trivial a simple vista.
		* Notese que el ultimo valor pasado a la funcion square (offset), tiene un factor aleatorio (entre 0 y 1),
		* que afecta a scale, permitiendo asi que la media calculada para una posicion pueda variar del valor exacto.
		*/
		for (y = 0; y <= this->max; y += half) {
			for (x = (y + half) % size; x <= this->max; x += size) {
				float r = ((float)rand() / (RAND_MAX));
				diamond(x, y, half, r * scale * 2 - scale);
			}
		}
		/*
		* Despues se calculan TODAS las medias (tipo diamond), para los puntos medios de los lados del la subdivision de 
		* tamaño size x size, esto es:
		*
		*	o-------=-------o		o---=---o---=---o			Los simbolos = y ! marcan las casillas sobre las que se 
		*	|       |       |		|   |   |   |   |			calculara la media diamond para la llamada actual
		*	|       |       |		!---X---!---X---!			Considerando los puntos o como casillas con valor de
		*	|       |       |		|   |   |   |   |			altura calculado
		*	!-------X-------!  -->	o---=---o---=---o			Los simbolos X son las casillas sobre las que se calculara
		*	|       |       |		|   |   |   |   |			la media square para la llamada actual
		*	|       |       |		!---X---!---X---!
		*	|       |       |		|   |   |   |   |
		*	o-------=-------o		o---=---o---=---o
		*
		* Siendo este el mapa, de tamaño (17 x 17) (se incluyen los puntos o), se estarian, en esta fase, calculando las medias
		* Para cuadrados  de (8 x 8), ya que la primera llamada a divide() se hace con el valor this->max, no con size (porque es
		* impar).
		* Si se sigue el algoritmo, se vera que para esta llamada inicial, solo se calcula el valor de la casilla marcada con X
		* (una media de tipo square), y luego se pasa a calcular las medias de los puntos marcados con = y !, tras hacerlo,
		* se llama a divide con size/2, y como se puede ver en el cuadrado de la derecha, los valores para las esquinas de cada
		* cuadrado de tamaño size/2 ya estan calculadas por la llamada anterior (representados con o)
		*/

		divide(size / 2);
	}

	/**
	* Se puede considerar una excepcion de divide(), se usa cuando se modifica un sector del mapa, se llama en generateSector()
	* donde se puede establecer la altura base del punto central, que influye en todas el terreno colindante.
	* Para esta primera llamada a divide() (la cual es divideSector), no se calcula la media para el punto medio del sector,
	* sino que se establece directamente al valor centralHeight, para el resto de puntos del sector, se sigue el algoritmo
	* normal de divide
	*
	* NO ES UNA FUNCION RECURSIVA, pero si que llama a divide, que si lo es
	*/
	void divideSector(int size, float centralHeight) {
		int x, y, half = size / 2;
		float scale = this->roughness * size;
		if (half < 1) return;	// por si se tratan secciones de 2x2
	
		this->set(half, half, centralHeight);	// La PRIMERA vez no se calcula una media square, se pone directamente este valor

		for (y = 0; y <= this->max; y += half) {
			for (x = (y + half) % size; x <= this->max; x += size) {
				float r = ((float)rand() / (RAND_MAX));
				diamond(x, y, half, r * scale * 2 - scale);
			}
		}
		divide(size / 2);	// Notese que la llamada es a divide, y no a divideSector()
	}

	

	
	
	/**
	* Devuelve un color en funcion de la altura, a diferencia de calculaColor(), los colores devueltos no son tan radicalmente
	* distintos, sino que se encuentran en una gama de grises, mas claros cuanta mayor altura
	*/
	/*
	COLORREF calculaColorSuave(int alto){
		COLORREF color;
		int valorA = 255 - alto;
		color = RGB(valorA, valorA, valorA);
		return color;
	}*/

	

public:

	/*
	* map es la matriz donde se guardan los valores de altura del terreno. Notese que no es un array bidimensional.
	* Para acceder a la posicion (x,y) de la matriz (se puede acceder con el metodo get()), seria:
	* map[x + size*y];
	*/
	float *map;

	// CONTRUCTORA SIN SEMILLA
	Map(int detail){
		this->size = pow(2,detail) +1;
		this->max = size - 1;
		this->map = new float[size * size];
		//this->altoMapa = 200;
		//this->alturaAgua = 3 * altoMapa / 5; // a partir de 3/5 de la altura hay agua
		this->seed = time(NULL);
		srand(this->seed);
		//this->hdc = GetDC(GetConsoleWindow()); // Get the DC from console
	}

	// CONSTRUCTORA CON SEMILLA
	Map(int detail, int seed){
		this->size = pow(2, detail) + 1;
		this->max = size - 1;
		this->map = new float[size * size];
		//this->altoMapa = 200;
		//this->alturaAgua = 3 * altoMapa / 5; // a partir de 3/5 de la altura hay agua
		this->seed = seed;
		srand(this->seed);
		//this->hdc = GetDC(GetConsoleWindow());
	}

	// METODOS PUBLICOS

	/**
	* Inicializa el mapa con los valores de altura (llamada a divide), y establece los valores higher y lower
	*/
	void generate(float roughness) {
		this->roughness = roughness;
		// Roughness, valor entre 0 y 1 (aunque puede ser > 1)
		this->set(0, 0, this->max * 3 / 4);
		this->set(this->max, 0, this->max * 3 / 4);
		this->set(this->max, this->max, this->max * 3 / 4);
		this->set(0, this->max, this->max * 3 / 4);
		/* 
		* Se pone un valor igual para todas las esquinas (size/3). Esto se puede variar, si se quiere, por ejemplo
		* un mapa que caiga o que tenga una elevacion hacia una o varia esquinas.
		*/

		divide(this->max);
	};

	/**
	* Inicializa un sector con los valores de altura (llamada a divideSector), y establece los valores higher y lower
	* Este metodo parte del hecho de que los valores ya estan establecidos.
	* Su uso se limita al ambito de modificaSector(), donde se crea un nuevo mapa, mas pequeño que el original, y sobre el
	* se recalcula un nuevo terreno.
	*/
	void generateSector(float roughness, int centralHeight) {
		this->roughness = roughness;
		// Roughness, valor entre 0 y 1 (aunque puede ser > 1)
		divideSector(this->max, centralHeight);
	};

	/**
	* Devuelve la semilla del mapa actual
	*/
	int getSeed(){
		return this->seed;
	}

	/**
	* Devuelve el mapa de alturas
	*/
	public:
	float* getMapa() const {
		return (this->map);
	}

	/**
	* Devuelve el tamaño del mapa
	*/
	int getSize() const { 
		return this->size;
	}

	/**
	* Referido a las distintas persepectivas desde las que se puede ver el mapa
	* De izquierda a derecha, frontalmente, o de derecha a izquierda
	*/
	enum Pers{
		LR,
		FRONT,
		RL
	};

	// TO-DO

	/**
	* Muestra el mapa desde la posicion indicada, con un grosor dado, desde la perspectiva indicada, y hasta los limites dados
	*/
	void muestraMapa(int desdeX, int desdeY, int grosor, Pers perspectiva, int limitX, int limitY){
	}

	/**
	* Modifica el sector comprendido entre los puntos (origX,origY) y (origX+1+2^lado,origY+1+2^lado), con un nuevo valor de roughness
	* y permitiendo establecer el valor del punto central del sector, para, por ejemplo, crear montañas o valles
	* Solo se pueden modificar sectores cuyo lado sea una potencia de dos, ya que el algoritmo Diamond-Square solo actua en 
	* matrices de lado (2^n + 1)
	* No se haran sobre matrices de menos de lado 3.
	*
	* LA FUNCION ESTA IMPLEMENTADA, PERO PROVOCA CAMBIOS MUY BRUSCOS EN EL TERRENO, CONVIENE REVISARLO
	*/
	void modificaSector(int origX, int origY, int lado, float roughness, float centralHeight){
		if (origX >= 0 && origX < size && origY >= 0 && origY < size){
			int tam = pow(2, lado) + 1;
			int destX = origX + tam;
			int destY = origY + tam;
			if (destX >= 0 && destX < size && destY >= 0 && destY < size){
				Map* modified = new Map(lado);
				for (int i = origX, iM = 0; i < destX; ++i, ++iM){
					for (int j = origY, jM = 0; j < destY; ++j, ++jM){
						modified->set(iM, jM, this->get(i, j));
					}
				}
				modified->generateSector(roughness,centralHeight);
				for (int i = origX, iM = 0; i < destX; ++i, ++iM){
					for (int j = origY, jM = 0; j < destY; ++j, ++jM){
						float l = modified->get(iM, jM);
						this->set(i, j, l);
					}
				}
				delete modified;
			}
		}
	}

};

#endif