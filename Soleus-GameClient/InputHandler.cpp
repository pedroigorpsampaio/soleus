#include "InputHandler.h"
#include <iostream>

InputHandler::InputHandler(): keyDown(false), keyUp (false), keyLeft(false), keyRight(false), closing(false), 
zooming(false), zoomDelta(0){}

int InputHandler::handleInput(sf::Event event) {

	// window closed
	if (event.type == sf::Event::Closed)
		this->closing = true;

	// key events
	if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased) {
		bool isPressed = event.type == sf::Event::KeyPressed ? true : false;
		if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S)
			this->keyDown = isPressed;
		if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W)
			this->keyUp = isPressed;
		if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::A)
			this->keyLeft = isPressed;
		if (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::D)
			this->keyRight = isPressed;
	}

	// mwheel events
	if (event.type == sf::Event::MouseWheelScrolled) {
		this->zooming = true;
		this->zoomDelta = event.mouseWheelScroll.delta;
	}
	else {
		this->zooming = false;
		this->zoomDelta = 0;
	}

	return 0;
}

bool InputHandler::isKeyUp() { return this->keyUp; }

bool InputHandler::isKeyDown() { return this->keyDown; }

bool InputHandler::isKeyLeft() { return this->keyLeft; }

bool InputHandler::isKeyRight() { return this->keyRight; }

bool InputHandler::isClosing() { return this->closing; }

bool InputHandler::isZooming() { return this->zooming; }

int InputHandler::getZoomDelta() { return this->zoomDelta; }
// reset all flags to stop any event going on
void InputHandler::halt() {
	this->keyUp = false;
	this->keyDown = false;
	this->keyLeft = false;
	this->keyRight = false;
	this->closing = false;
	this->zooming = false;
	this->zoomDelta = 0;
}
