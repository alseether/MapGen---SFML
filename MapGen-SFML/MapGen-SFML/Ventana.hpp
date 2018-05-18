#ifndef VENTANA_HPP
#define VENTANA_HPP

#include <SFML/Graphics.hpp>
#include <string>
class Ventana {

private:

	std::string name;
	sf::RenderWindow v; 

public:

	Ventana(std::string n) :
		v(sf::VideoMode::getDesktopMode(), name),
		name(n){

	};

	void close(){
		v.close();
	}

	void show(sf::VertexArray bitmap){
		v.create(sf::VideoMode::getDesktopMode(), name);
		while (v.isOpen()){
			sf::Event event;
			while (v.pollEvent(event)){
				if (event.type == sf::Event::Closed)
					v.close();
			}
			v.clear();
			v.draw(bitmap);
			v.display();
		}
	}


};

#endif
