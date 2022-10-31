#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include "TileMap.h"
#include <iostream>
#include <sstream>
#include "InputHandler.h"
#include "Player.h"
#include "Entity.h"
#include "Networker.h"
#include "GameLoop.h"

#ifndef GAME_H
#define GAME_H

// Class that manages the game on client-side
class Game : public GameLoop
{
	public:
		Game(); // constructor

		//====================
		// Interface
		//====================

		/// Called after construction, immediately before entering the update loop.
		/// Connection establishment and resource loading should go here. If this
		/// method returns false, the Game will not enter the update loop, and it
		/// will immediately switch to the Shutdown state.
		virtual bool load() override;

		/// Called after the update loop has finished. Frees any resources used by
		/// the game before exiting.
		virtual void shutdown() override;

		/// Called on every tick. Because we're running at a fixed time-step, this
		/// should be called at a regular interval.
		/// @param    dt    The time since the last update (seconds).
		virtual void update(float dt) override;

		/// Draw the Game
		virtual void draw() override;
	//private:
		void sendPacketToServer(int messageType); /// Prepares the different types of packets and send to server 
		void sendPingToServer(); /// sends ping packet to the server
};

// handles packets received - called as a callback from network object
void handlePacket(sf::Packet packet, sf::IpAddress sender, unsigned short port);
// sync client clock with server clock
void syncClock(sf::Packet packet);

#endif