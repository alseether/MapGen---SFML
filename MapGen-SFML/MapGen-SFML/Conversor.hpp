#ifndef CONVERSOR_HPP
#define CONVERSOR_HPP

#include <SFML\Graphics.hpp>
#include "Map.hpp"

class Conversor {
private:
	float* map;
	int size;
	int altoMapa;
	int alturaAgua;
	float higher, lower;

	/**
	* Devuelve el valor mas alto del mapa
	*/
	float findHigher(){
		float mayor = 0;
		for (int i = 0; i < this->size*this->size; ++i){
			if (map[i] > mayor){
				mayor = map[i];
			}
		}
		return mayor;
	}

	/**
	* Devuelve el valor mas bajo del mapa (Puede ser menor que 0)
	*/
	float findLower(){
		float menor = size;
		for (int i = 0; i < this->size*this->size; ++i){
			if (map[i] < menor){
				menor = map[i];
			}
		}
		return menor;
	}

	/**
	* Devuelve el valor representativo de altura de una casilla entre 0 - altoMapa
	* Para lower el valor devuelto sera 0
	* Para higher el valor devuelto sera altoMapa;
	*/
	int calculaAlto(float n){
		int tamRangoAlturas = higher - lower;	// El rango de alturas siempre sera higher-lower, incluso si lower < 0
		return (tamRangoAlturas == 0) ? 200 : (n - lower) * 200 / tamRangoAlturas;
	}

	/**
	* Devuelve el color del agua correspondiente a un valor de altura (representativo, entre alturaAgua-altoMapa)
	*/
	sf::Color calculaColorAgua(int alto){
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
	sf::Color calculaColor(int alto){
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
			valorA = 255 - (alto * 45 / (altoMapa/7));
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
			int altoMax = 3 *  altoMapa / 7;
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
			valorA = 100 - ((alto - altoMin) * 50 / (altoMax -altoMin));
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
			valorA = 40 - ((alto-altoMin) * 25 / (altoMax -altoMin));
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

public:
	Conversor(const Map &mapa) : 
		size(mapa.getSize()),
		altoMapa(200),
		alturaAgua(3 * altoMapa / 5){

		map = mapa.map;
		higher = findHigher();
		lower = findLower();
	};

	/**
	* Muestra el mapa en 2D, como una vista de planta (desde arriba), con un ancho y alto de pixel dados
	* La llamada a la funcion sin argumentos establece un alto y un  ancho de 5 pixeles por cada valor del
	* mapa.
	*/
	sf::VertexArray getVistaPlanta(int pixelWidth){
		int realSize = size*pixelWidth;
		sf::VertexArray ret(sf::PrimitiveType::Points, realSize*realSize);
		int x, y;
		int alto;
		sf::Color color;
		for (int i = 0; i < size*size; i++)
		{
			alto = calculaAlto(map[i]);
			color = calculaColor(alto);
			x = (i % size);
			y = (i / size);
			for (int j = 0; j < pixelWidth; ++j){
				// Bucle recorre las x
				for (int k = 0; k < pixelWidth; ++k){
					// Bucle recorre las y
					int realX = (x*pixelWidth) + j;
					int realY = (y*pixelWidth) + k;
					ret[realX + realSize * realY].position.x = realX;
					ret[realX + realSize * realY].position.y = realY;
					ret[realX + realSize * realY].color = color;
				}
			}
		}
		return ret;
	}


	/**
	* Muestra una perspectiva rellena de izquierda a derecha (LR -- LeftToRight) del mapa.
	* La llamada sin argumentos establece que se muestra desde el principio de la ventana, con un grosor de linea de 1
	* (sin espaciado entre lineas)
	*/
	sf::VertexArray getCorte3DLR(int pixelWidth){
		sf::VertexArray ret(sf::PrimitiveType::Lines, 2 * size * size * pixelWidth);
		int x, y; 
		sf::Color color;
		int offset = -1;
		int alto;
		for (int i = 0; i < size*size; ++i)
		{
			if (i%size == 0){
				offset++;
			}
			alto = calculaAlto(map[i]);
			x = (i % size);
			y = (i / size);
			if (alto > alturaAgua){
				color = calculaColorAgua(alto);
				alto = alturaAgua;
			}
			else{
				color = calculaColor(alto);
			}
			for (int j = 0; j < pixelWidth; ++j){
				int index = (i * 2 * pixelWidth) + 2 * j;
				ret[index].position.x = (x * pixelWidth + j) + offset;
				ret[index].position.y = alto + offset;
				ret[index].color = color;
				ret[index + 1].position.x = (x * pixelWidth + j) + offset;
				ret[index + 1].position.y = altoMapa + offset;
				ret[index + 1].color = color;
			}
		}
		return ret;
	}

	/**
	* Muestra una perspectiva a puntos de izquierda a derecha (LR -- LeftToRight) del mapa.
	* La llamada sin argumentos establece que se muestra desde el principio de la ventana, con un grosor de linea de 1
	* (sin espaciado entre lineas)
	*/
	sf::VertexArray getCorte3DLRDotted(int pixelWidth){
		sf::VertexArray ret(sf::PrimitiveType::Points, size * size * pixelWidth);
		int x, y;
		sf::Color color;
		int offset = -1;
		int alto;
		for (int i = 0; i < size*size; ++i)
		{
			if (i%size == 0){
				offset++;
			}
			alto = calculaAlto(map[i]);
			x = (i % size);
			y = (i / size);
			if (alto > alturaAgua){
				color = calculaColorAgua(alto);
				alto = alturaAgua;
			}
			else{
				color = calculaColor(alto);
			}
			for (int j = 0; j < pixelWidth; ++j){
				int index = (i * pixelWidth) + j;
				ret[index].position.x = (x * pixelWidth + j) + offset;
				ret[index].position.y = alto + offset;
				ret[index].color = color;
			}
		}
		return ret;
	}

	/**
	* Muestra una perspectiva rellena de derecha a izquierda (RL -- RightToLeft) del mapa.
	* La llamada sin argumentos establece que se muestra desde el principio de la ventana, con un grosor de linea de 1
	* (sin espaciado entre lineas)
	*/

	sf::VertexArray getCorte3DRL(int pixelWidth){
		sf::VertexArray ret(sf::PrimitiveType::Lines, 2 * size * size * pixelWidth);
		int x, y;
		sf::Color color;
		int offset = size + 1;
		int alto;
		for (int i = 0; i < size*size; ++i)
		{
			if (i%size == 0){
				offset--;
			}
			alto = calculaAlto(map[i]);
			x = (i % size);
			y = (i / size);
			if (alto > alturaAgua){
				color = calculaColorAgua(alto);
				alto = alturaAgua;
			}
			else{
				color = calculaColor(alto);
			}
			for (int j = 0; j < pixelWidth; ++j){
				int index = (i * 2 * pixelWidth) + 2 * j;
				ret[index].position.x = (x * pixelWidth + j) + offset;
				ret[index].position.y = alto + size-offset;
				ret[index].color = color;
				ret[index + 1].position.x = (x * pixelWidth + j) + offset;
				ret[index + 1].position.y = altoMapa + size-offset;
				ret[index + 1].color = color;
			}
		}
		return ret;
	}

	/**
	* Muestra una perspectiva a puntos de derecha a izquierda (RL -- RightToLeft) del mapa.
	* La llamada sin argumentos establece que se muestra desde el principio de la ventana, con un grosor de linea de 1
	* (sin espaciado entre lineas)
	*/

	sf::VertexArray getCorte3DRLDotted(int pixelWidth){
		sf::VertexArray ret(sf::PrimitiveType::Lines, size * size * pixelWidth);
		int x, y;
		sf::Color color;
		int offset = size + 1;
		int alto;
		for (int i = 0; i < size*size; ++i)
		{
			if (i%size == 0){
				offset--;
			}
			alto = calculaAlto(map[i]);
			x = (i % size);
			y = (i / size);
			if (alto > alturaAgua){
				color = calculaColorAgua(alto);
				alto = alturaAgua;
			}
			else{
				color = calculaColor(alto);
			}
			for (int j = 0; j < pixelWidth; ++j){
				int index = (i * pixelWidth) + j;
				ret[index].position.x = (x * pixelWidth + j) + offset;
				ret[index].position.y = alto + size - offset;
				ret[index].color = color;
			}
		}
		return ret;
	}

	/**
	* Muestra una perspectiva rellena frontal del mapa.
	* La llamada sin argumentos establece que se muestra desde el principio de la ventana, con un grosor de linea de 1
	* (sin espaciado entre lineas)
	*/
	sf::VertexArray getCorte3DFront(int pixelWidth){
		sf::VertexArray ret(sf::PrimitiveType::Lines, 2 * size * size * pixelWidth);
		int x, y;
		int offset = -1;
		sf::Color color;
		int alto;
		for (int i = 0; i < size*size; ++i)
		{
			if (i%size == 0){
				offset++;
			}
			alto = calculaAlto(map[i]);
			x = (i % size);
			y = (i / size);
			if (alto > alturaAgua){
				color = calculaColorAgua(alto);
				alto = alturaAgua;
			}
			else{
				color = calculaColor(alto);
			}
			for (int j = 0; j < pixelWidth; ++j){
				int index = (i * 2 * pixelWidth) + 2 * j;
				ret[index].position.x = (x * pixelWidth + j);
				ret[index].position.y = alto + offset;
				ret[index].color = color;
				ret[index + 1].position.x = (x * pixelWidth + j);
				ret[index + 1].position.y = altoMapa + offset;
				ret[index + 1].color = color;
			}
		}
		return ret;
	}

	/**
	* Muestra una perspectiva a puntos frontal del mapa.
	* La llamada sin argumentos establece que se muestra desde el principio de la ventana, con un grosor de linea de 1
	* (sin espaciado entre lineas)
	*/
	sf::VertexArray getCorte3DFrontDotted(int pixelWidth){
		sf::VertexArray ret(sf::PrimitiveType::Lines, size * size * pixelWidth);
		int x, y;
		int offset = -1;
		sf::Color color;
		int alto;
		for (int i = 0; i < size*size; ++i)
		{
			if (i%size == 0){
				offset++;
			}
			alto = calculaAlto(map[i]);
			x = (i % size);
			y = (i / size);
			if (alto > alturaAgua){
				color = calculaColorAgua(alto);
				alto = alturaAgua;
			}
			else{
				color = calculaColor(alto);
			}
			for (int j = 0; j < pixelWidth; ++j){
				int index = (i * pixelWidth) + j;
				ret[index].position.x = (x * pixelWidth + j);
				ret[index].position.y = alto + offset;
				ret[index].color = color;
			}
		}
		return ret;
	}

	/**
	* Muestra los distintos colores que se utilizan para cada altura del mapa
	* La llamada sin argumentos lo muestra al comienzo de la pantalla
	*/
	sf::VertexArray getEscala(){
		sf::VertexArray ret(sf::PrimitiveType::Lines, 2 * altoMapa);
		sf::Color color;
		for (int j = 0; j < altoMapa; ++j){
			color = calculaColor(j);
			ret[2 * j].position.x = 10;
			ret[2 * j].position.y = 10 + j;
			ret[2 * j].color = color;
			ret[2 * j +1].position.x = 1000;
			ret[2 * j +1].position.y = 10 + j;
			ret[2 * j +1].color = color;
		}
		return ret;
	}
};

#endif