#include <iostream>
#include <iomanip>
#include <math.h>
#include <Windows.h>
#include <windows.system.h>
#include "Map.hpp"
#include "Conversor.hpp"
#include "Ventana.hpp"

using namespace std;

int main() {
	// Create window object
	sf::RenderWindow window(sf::VideoMode(600, 600), "MountDet");

	sf::View view(sf::FloatRect(0, 0, window.getSize().x, window.getSize().y));
	view.setCenter(window.getSize().x / 2, window.getSize().y / 2);
	float zoom = 1;
	view.zoom(zoom);

	std::vector<sf::CircleShape> circles;

	// Set window frame rate
	window.setFramerateLimit(60);

	Map m(8);

	sf::Font f;
	f.loadFromFile("C:/Windows/Fonts/Arial.ttf");

	sf::Text t;
	t.setFont(f);
	t.setPosition(10, 10);

	float alpha = 0;
	m.generate(7);
	bool rightButClicked = false;
	int refRot = -1;
	bool leftButClicked = false;
	sf::Vector2f refMove;
	int thres = 200;
	float angle = 0;
	// Main window loops
	while (window.isOpen()) {
		// Create events object
		sf::Event event;
		// Loop through events
		while (window.pollEvent(event)) {
			// Close window
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if (event.type == sf::Event::MouseButtonPressed){
				if (event.mouseButton.button == sf::Mouse::Right){
					rightButClicked = true;
					refRot = sf::Mouse::getPosition(window).x;
				}
				else if (event.mouseButton.button == sf::Mouse::Left){
					leftButClicked = true;
					refMove = sf::Vector2f(sf::Mouse::getPosition(window));
				}
			}
			else if (event.type == sf::Event::MouseButtonReleased){
				if (event.mouseButton.button == sf::Mouse::Right){
					rightButClicked = false;
				}
				else if (event.mouseButton.button == sf::Mouse::Left){
					leftButClicked = false;
				}

			}
			else if (event.type == sf::Event::MouseWheelScrolled){
				if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel){
					zoom = 1 - (0.25 * event.mouseWheelScroll.delta);
					std::cout << "zoom: " << zoom << std::endl;
					view.zoom(zoom);
				}
			}
			else if (event.type == sf::Event::KeyPressed){
				auto k = event.key.code;
				switch (k){
				case sf::Keyboard::A:
					sf::CircleShape cs(3);
					cs.setOutlineColor(sf::Color::Red);
					cs.setPosition(sf::Vector2f(sf::Mouse::getPosition(window)));
					circles.push_back(cs);
					break;
				}
			}
		}

		t.setString("Angle: " + std::to_string(m.getAngle()));

		if (leftButClicked){
			sf::Vector2f mousePos(sf::Mouse::getPosition(window)), diff;
			diff = refMove - mousePos;
			refMove = mousePos;
			view.move(diff);
		}
		if (rightButClicked){
			int mouseX = sf::Mouse::getPosition(window).x;
			angle = refRot - mouseX;
			refRot = mouseX;
			m.rotate(angle);
		}
		// Clear window
		window.clear(sf::Color::Black);
		window.setView(view);
		window.draw(m);
		//window.draw(t);

		for (auto &c : circles){
			window.draw(c);
		}
		// Display window contents
		window.display();
	}

	return 0;
}
