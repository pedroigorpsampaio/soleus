#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <sstream>
#include "Networker.h"
#include "GameLoop.h"

#ifndef SERVER_H
#define SERVER_H

// Class that manages the server
class Server : public GameLoop
{
public:
	Server(); // constructor

	//====================
	// Interface
	//====================

	/// Called after construction, immediately before entering the update loop.
	/// Connection establishment and resource loading should go here. If this
	/// method returns false, the Server will not enter the update loop, and it
	/// will immediately switch to the Shutdown state.
	virtual bool load() override;

	/// Called after the update loop has finished. Frees any resources used by
	/// the server before exiting.
	virtual void shutdown() override;

	/// Called on every tick. Because we're running at a fixed time-step, this
	/// should be called at a regular interval.
	/// @param    dt    The time since the last update (seconds).
	virtual void update(float dt) override;

	/// Draw the Server Console
	virtual void draw() override;
};

// handles packets received - called as a callback from network object
void handlePacket(sf::Packet packet, sf::IpAddress sender, unsigned short port);
void sendSnapshot();

#endif