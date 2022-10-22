#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include "TileMap.h"
#include <iostream>
#include <sstream>
#include "InputHandler.h"
#include "Player.h"
#include "Entity.h"
#include <chrono>

#ifndef GAME_H
#define GAME_H

// Class that manages the game on client-side
class Game
{
	public:
		Game(); // constructor
		void run(); // starts the game

	private:
		// load connection
		int loadConnection();

		// loads configurations
		void load();

		// updates game
		void update(float dt);

		// draw game
		void draw();

		// sends udp packet to a server
		int sendUdpPacket(sf::Packet packet, sf::IpAddress recipient, unsigned short port);

		// handles messages received via udp on socket
		void receiveUdpPacket(float dt);

		// sends ping packet to server to measure latency
		void sendPingToServer();
};

#endif