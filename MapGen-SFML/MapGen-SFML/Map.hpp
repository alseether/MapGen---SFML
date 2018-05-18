#ifndef MAP_HPP
#define MAP_HPP

#include <SFML\Graphics.hpp>
#include <iostream>
#include <iomanip>
#include <set>
#include <hash_map>
#include <math.h>
#include <time.h>
#include <Windows.h>

class Map : public sf::Drawable, sf::Transformable {
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

	int maxHeight;
	int minHeight;

	int peekHeight;
	sf::VertexArray va;
	float angle;
	int altoMapa;
	int alturaAgua;

	std::vector<sf::Color> grads;

	std::vector<float> steps;

	// METODOS PRIVADOS

	/**
	* Obtiene el valor de la posicion (x,y) del mapa. Devuelve -1 si la posicion es invalida y el valor en caso contrario
	*/
	float get(int x, int y) const{
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
		if (ave + offset > this->maxHeight){
			this->maxHeight = ave + offset;
		}
		if (ave + offset < this->minHeight){
			this->minHeight = ave + offset;
		}
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
		if (ave + offset > this->maxHeight){
			this->maxHeight = ave + offset;
		}
		if (ave + offset < this->minHeight){
			this->minHeight = ave + offset;
		}
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

	void normalize(){
		int diff = 0 - this->minHeight;
		this->minHeight = 0;
		this->maxHeight = INT_MIN;
		for (size_t i = 0; i < size; ++i){
			for (size_t j = 0; j < size; ++j){
				int v = this->get(i, j);
				this->set(i, j, v + diff);
				if (v + diff > this->maxHeight){
					this->maxHeight = v + diff;
				}
			}
		}
		int maxval = this->maxHeight;
		for (size_t i = 0; i < size; ++i){
			for (size_t j = 0; j < size; ++j){
				int v = (this->get(i, j) * 255) / maxval;
				this->set(i, j, v);
				if (v > this->maxHeight){
					this->maxHeight = v;
				}
			}
		}
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

	sf::Color calculaColorAgua(int altura){
		int alto = 255 - altura;
		sf::Color color;
		int valorA, valorB;
		/*
		* Gama de azules entre RGB(3,35,239) a RGB(3,11,78)
		* Valores de alto: desde alturaAgua a altoMapa
		* Valores de verde: desde 35 a 11 (24 valores)
		* Valores de azul: desde 239 a 78 (161 valores)
		*/
		valorA = 35 - ((alto - alturaAgua) * 24 / (altoMapa - alturaAgua));
		valorB = 239 - ((alto - alturaAgua) * 161 / (altoMapa - alturaAgua));
		color = sf::Color::Color(3, valorA, valorB, 255);
		return color;
	}

	/**
	* Devuelve el color correspondiente a un valor de altura (representativo, entre 0-altoMapa)
	*/
	sf::Color calculaColor(int altura){
		int alto = 255 - altura;
		sf::Color color;
		int offset = (alto * 128 / altoMapa);
		/*
		* offset aplica una variacion al color base, para que alturas diferentes en un rango,
		* tengan distinta intensidad de ese mismo color, haciendolo mas visual y realista
		*/
		int valorA = 128 + offset;
		int valorB = 128 - offset;
		if (alto < altoMapa / 7){
			/*
			* Blancos para las cimas
			* Valores de alto: de 0  a altoMapa/7
			* Valores de blancos: de 255 a 210
			* La relacion se calcula con una regla de 3
			*/
			valorA = 255 - (alto * 45 / (altoMapa / 7));
			color = sf::Color::Color(valorA, valorA, valorA, 255);
		}
		else if (alto < 2 * altoMapa / 7){
			/*
			* Grises claros para laderas de montaña
			* Valores de alto: de altoMapa/7 a 2 * altoMapa/7
			* Valores de gris: de 140 a 64 (76 valores)
			*/
			int altoMin = altoMapa / 7;
			int altoMax = 2 * altoMapa / 7;
			valorA = 140 - ((alto - altoMin) * 76 / (altoMax - altoMin));
			color = sf::Color::Color(valorA, valorA, valorA, 255);
		}
		else if (alto < 3 * altoMapa / 7){
			/*
			* Verdes oscuros para pies de montaña, simulando bosques
			* Valores de alto: de 2 * altoMapa/7 a 3 * altoMapa/7
			* Valores de verde: de 64 a 128 (64 valores)
			*/
			int altoMin = 2 * altoMapa / 7;
			int altoMax = 3 * altoMapa / 7;
			valorA = 64 + ((alto - altoMin) * 64 / (altoMax - altoMin));
			color = sf::Color::Color(0, valorA, 0, 255);
		}
		else if (alto < 4 * altoMapa / 7){
			/*
			* Amarillos claros para simular arena de desiertos o playas
			* Valores de alto: de 3*altoMapa/7 a 4*altoMapa/7
			* Valores de verde: de 223 a 255 (32 valores)
			*/
			int altoMin = 3 * altoMapa / 7;
			int altoMax = 4 * altoMapa / 7;
			valorA = 223 + ((alto - altoMin) * 32 / (altoMax - altoMin));
			color = sf::Color::Color(255, valorA, 128, 255);
		}
		else if (alto < 5 * altoMapa / 7){
			/*
			* Marrones oscuros para barro cerca del agua
			* Valores de alto: de 4*altoMapa/7 a 5*altoMapa/7
			* Valores de rojo: de 100 a 50
			* Valores de verde/azul: de 48 a 24 (24 valores)
			*/
			int altoMin = 4 * altoMapa / 7;
			int altoMax = 5 * altoMapa / 7;
			valorA = 100 - ((alto - altoMin) * 50 / (altoMax - altoMin));
			valorB = 48 - ((alto - altoMin) * 24 / (altoMax - altoMin));
			color = sf::Color::Color(valorA, valorB, valorB, 255);
		}
		else if (alto < 6 * altoMapa / 7){
			/*
			* Grises y negros para zonas pantanosas profundas
			* Valores de alto: de 5*altoMapa/7 a 6*altoMapa/7
			* Valores de gris: de 40 a 15 (25 valores)
			*/
			int altoMin = 5 * altoMapa / 7;
			int altoMax = 6 * altoMapa / 7;
			valorA = 40 - ((alto - altoMin) * 25 / (altoMax - altoMin));
			color = sf::Color::Color(valorA, valorA, valorA, 255);
		}
		else{	// (alto < altoMapa)
			/*
			* "Casi" negro para zonas muy profundas
			* Valores de alto: 6*altoMapa/7 a altoMapa
			* Valores de negro: 3
			*/
			color = sf::Color::Color(10, 10, 10, 255);
		}
		return color;
		/*
		* La eleccion de los colores no sigue ningun patron, he ido probando combinaciones hasta encontrar colores
		* bastante diferentes entre capas, pero que tambien varien de intensidad dentro de la misma capa
		* Los colores por capas se pueden ver usando el metodo mostrarEscala()
		* NOTA: Si, ya se que los colores son una mierda, sois libres de cambiarlos y si encontrais una combinacion mejor,
		* hacedmelo saber
		*/
	}

	sf::Vector2f iso(int x, int y) const{
		return sf::Vector2f(0.5*(size + x - y), 0.5*(x + y));
	}
	sf::Vector2f iso(sf::Vector2f v) const{
		return iso(v.x, v.y);
	}

	sf::Vector2f perspective(int x, int y, int z) const{
		auto point = iso(x, y);
		auto x0 = size * 0.5;
		auto y0 = size * 0.2;
		auto pz = size * 0.5 - z + point.y * 0.75;
		auto px = (point.x - size * 0.5) * 6;
		auto py = (size - point.y)*0.005 + 1;

		return sf::Vector2f((x0 + px / py), (y0 + pz / py));
	}

	void calculateVertex(){
		va.clear();
		int initXOff = 50;
		int initYOff = 50;
		int margin = 1;
		int offY = 0;
		//sf::Vector2f ctr(initXOff + size / 2, initYOff + size / 2);
		sf::Vector2f ctr = perspective(initXOff + size / 2, initXOff + size / 2, 0);
		for (size_t i = 0; i < size; ++i){
			for (size_t j = 0; j < size; ++j){
				int h = this->get(i, j);
				sf::Color c = calculaColor(h);
				if (h < alturaAgua){
					c = calculaColorAgua(h);
				}

				float r = (-angle * 3.14159265359) / 180;
				float m = ctr.x;
				float n = ctr.y;
				int x = initXOff;
				x += (std::cos(r) * (i - m) + (j - n)*std::sin(r) + m);

				int y = initYOff + j;


				y = initYOff + offY;
				y += -std::sin(r) * (i - m) + std::cos(r)*(j - n) + n;

				auto val = get(i, j);
				auto top = perspective(x, y, val);
				auto bottom = perspective(x + 1, y, 0);

				if (h < alturaAgua){
					top = perspective(x, y, alturaAgua);
				}

				sf::Vertex v3(top, c);
				va.append(v3);
				sf::Vertex v4(bottom, c);
				va.append(v4);
			}
		}
		if (angle >180){
			int n = va.getVertexCount();
			sf::Vertex vaux;
			for (int i = 0; i < n / 2; ++i){
				vaux = va[i];
				va[i] = va[n - 1 - i];
				va[n - 1 - i] = vaux;
			}
		}

	}

public:

	/*
	* map es la matriz donde se guardan los valores de altura del terreno. Notese que no es un array bidimensional.
	* Para acceder a la posicion (x,y) de la matriz (se puede acceder con el metodo get()), seria:
	* map[x + size*y];
	*/
	float *map;

	// CONTRUCTORA SIN SEMILLA
	Map(int detail) :
		angle(0),
		size(pow(2, detail) + 1),
		va(sf::PrimitiveType::Lines, size*size * 2)
	{
		//this->size = pow(2,detail) +1;
		this->max = size - 1;
		this->maxHeight = INT_MIN;
		this->minHeight = INT_MAX;
		this->map = new float[size * size];
		//this->altoMapa = 200;
		//this->alturaAgua = 3 * altoMapa / 5; // a partir de 3/5 de la altura hay agua
		this->seed = time(NULL);
		srand(this->seed);
		//this->hdc = GetDC(GetConsoleWindow()); // Get the DC from console
	}

	// CONSTRUCTORA CON SEMILLA
	Map(int detail, int seed) :
		angle(0),
		size(pow(2, detail) + 1),
		va(sf::PrimitiveType::Lines, size*size * 2)
	{
		//this->size = pow(2, detail) + 1;
		this->max = size - 1;
		this->maxHeight = INT_MIN;
		this->minHeight = INT_MAX;
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

		grads = {
			// Bottom color				// Top Color
			sf::Color(165, 88, 11), sf::Color(196, 109, 23),
			sf::Color(214, 183, 62), sf::Color(165, 88, 11),
			sf::Color(183, 182, 179), sf::Color(196, 195, 192),
			sf::Color(53, 130, 23), sf::Color(67, 150, 34),
			sf::Color(139, 165, 153), sf::Color(160, 186, 173),
			sf::Color(224, 224, 224), sf::Color(250, 250, 250),
		};
		steps = { 0, 20, 40, 80, 120, 200, 255 };

		altoMapa = 255;
		alturaAgua = (2 * altoMapa / 5);

		divide(this->max);
		normalize();
		calculateVertex();

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

	float getAngle() const{
		return angle;
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

	void setThreshold(int t){
		peekHeight = t;
	}

	virtual void rotate(float angle){
		this->angle += angle;
		while (this->angle > 360){
			this->angle -= 360;
		}
		while (this->angle < -360){
			this->angle += 360;
		}
		calculateVertex();
	}

	virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const{
		/*
		sf::VertexArray va(sf::PrimitiveType::Points, size*size);
		for (size_t i = 0; i < size; ++i){
		for (size_t j = 0; j < size; ++j){
		int h = this->get(i, j);
		sf::Color c(h, h, h);
		if (peeks.find(j * 10000 + i) != peeks.end()){
		c = sf::Color::Blue;
		}
		sf::Vertex v(sf::Vector2f(i,j), c);
		va.append(v);
		}
		}

		*/
		sf::CircleShape ct(1);
		ct.setOrigin(0.5, 0.5);
		ct.setOutlineThickness(1);
		ct.setOutlineColor(sf::Color::Green);
		ct.setPosition(50 + size / 2, 50 + size / 2);
		target.draw(va);
		target.draw(ct);
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
				modified->generateSector(roughness, centralHeight);
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