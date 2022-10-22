#include <SFML\Window\Event.hpp>
#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

class InputHandler {
	
	private:
		bool keyUp; // is the key for up movement being pressed?
		bool keyDown;  // is the key for down movement being pressed?
		bool keyLeft;  // is the key for left movement being pressed?
		bool keyRight;  // is the key for right movement being pressed?
		bool closing; // is the window to be closed?
		bool zooming; // is player zooming
		int zoomDelta; // which direction player is zooming (in/out)

	public:
		InputHandler(); // constructor that sets all inputs initial state
		int handleInput(sf::Event); // handles input events
		bool isKeyUp(); // returns if the key for up movement is being pressed
		bool isKeyDown(); // returns if the key for down movement is being pressed
		bool isKeyLeft(); // returns if the key for left movement is being pressed
		bool isKeyRight(); // returns if the key for right movement is being pressed
		bool isClosing(); // return if the window is to be closed?
		bool isZooming(); // is player zooming
		int getZoomDelta(); // get integer representing which direction player is zooming (in/out)
		void halt(); // halts all events (reset flags)
};

#endif