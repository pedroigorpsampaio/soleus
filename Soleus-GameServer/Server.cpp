#include "Server.h"
#include <thread>

// timers
std::chrono::time_point<std::chrono::system_clock> timer = std::chrono::system_clock::now();

sf::RenderWindow window; // server window
sf::Clock deltaClock;	// server delta clock
sf::Font font; 		// server main font

// server views
sf::View consoleView;

// Network object that bridges communication with server
Networker net;

// constructor
Server::Server() {}

// loads server configurations - run at the start of loop cycle
bool Server::load() {
	window.create(sf::VideoMode(640, 400), "Server Console");
	window.setVerticalSyncEnabled(true);

	return true;
}

// shutdown - run after loop ends
void Server::shutdown() {
	window.close();
}


// updates server each loop cycle
void Server::update(float dt) {
	// observes received messages - calls callback when packet is received
	net.observe(handlePacket);

	sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			window.close();
	}
}

// draws game - parte of gameloop
void Server::draw() {
	window.clear();
	// draw console
	window.display();
}

// handles packets received - called as a callback from network object
void handlePacket(sf::Packet packet, sf::IpAddress sender, unsigned short port) {
	using namespace std::this_thread;     // sleep_for, sleep_until
	using namespace std::chrono_literals; // ns, us, ms, s, h, etc.

	int messageType;

	//sleep_for(100ms);

	packet >> messageType;

	if (messageType == Message::Ping) {
		/*sf::Uint16 x = 1;*/
		//std::string response = "pong";

		sf::Packet reply;
		reply << Message::Ping;

		//std::cout << sender << ":" << clientPort;

		// sends packet reply to client
		net.sendUdpPacket(reply, sender, Properties::ClientPort);
	}
}