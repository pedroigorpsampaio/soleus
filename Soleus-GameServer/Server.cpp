#include "Server.h"
#include <thread>
#include "Util.h"

// timers
std::chrono::time_point<std::chrono::system_clock> timer = std::chrono::system_clock::now();

sf::RenderWindow window; // server window
sf::Clock deltaClock;	// server delta clock
sf::Font font; 		// server main font

// server views
sf::View consoleView;	

// server-time text
sf::Text svTimeText;

// Network object that bridges communication with server
Networker net;

// constructor
Server::Server() {}

// player moving data
bool right = false, left = false, up = false, down = false;
sf::Vector2f playerPos(INIT_X, INIT_Y);
double snapshotTimer = 9999;
bool stateChanged = false;

// loads server configurations - run at the start of loop cycle
bool Server::load() {
	window.create(sf::VideoMode(640, 400), "Server Console");
	window.setVerticalSyncEnabled(true);

	// load game main font
	if (!font.loadFromFile("assets\\fonts\\arial.ttf")) {
		std::cout << "unable to load game main font" << std::endl;
	}

	// select the font
	svTimeText.setFont(font); // font is a sf::Font
	// set the string to display
	svTimeText.setString("Server-Time: ");
	// set the character size
	svTimeText.setCharacterSize(24); // in pixels, not points!
	// set the color
	svTimeText.setFillColor(sf::Color::White);
	// set the text style
	svTimeText.setStyle(sf::Text::Bold);

	return true;
}

// shutdown - run after loop ends
void Server::shutdown() {
	window.close();
}


// updates server each loop cycle
void Server::update(float dt) {
	sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			window.close();
	}
	
	// updates player position
	// gets input first to normalize it before applying movement (avoid faster diagonals)
	sf::Vector2f direction(0, 0);
	if (down)
		direction.y = 1;
	if (up)
		direction.y = -1;
	if (left)
		direction.x = -1;
	if (right)
		direction.x += 1;

	sf::Vector2f nDirection = util::normalize(direction);
	//std::cout << nDirection.x << "," << nDirection.y << std::endl;
	playerPos.x += nDirection.x * PLAYER_SPEED * dt;
	playerPos.y += nDirection.y * PLAYER_SPEED * dt;
	//std::cout << playerPos.x << "," << playerPos.y << std::endl;
	//if (down)
	//	playerPos.y += PLAYER_SPEED * dt;
	//if (up)
	//	playerPos.y -= PLAYER_SPEED * dt;
	//if (left)
	//	playerPos.x -= PLAYER_SPEED * dt;
	//if (right)
	//	playerPos.x += PLAYER_SPEED * dt;

	// sends snapshot to client if it is time
	if (snapshotTimer > (1.0f / SNAPSHOT_TICKRATE)) {
		sendSnapshot();
		snapshotTimer = 0;

		// updates server time
		using namespace std::chrono;
		// Get current time with precision of milliseconds
		auto now = time_point_cast<milliseconds>(system_clock::now());
		// sys_milliseconds is type time_point<system_clock, milliseconds>
		using sys_milliseconds = decltype(now);
		// Convert time_point to signed integral type
		auto svTimestamp = now.time_since_epoch().count();
		// Convert signed integral type to time_point
		sys_milliseconds time{ sys_milliseconds::duration{svTimestamp} };

		// updates server-time
		std::stringstream ss;
		ss << "Server-Time: " << getDateTime(time) << std::endl;
		std::string svTimeStr = ss.str();
		svTimeText.setString(svTimeStr);
	}

	// updates timers
	snapshotTimer += dt;
	// observes received messages - calls callback when packet is received
	net.observe(handlePacket);
}

// draws game - parte of gameloop
void Server::draw() {
	window.clear();
	// draw console
	window.draw(svTimeText);
	// stop drawing
	window.display();
}

void sendSnapshot() {

	sf::Packet snapshotPacket;
	snapshotPacket << Message::GameSync << playerPos.x << playerPos.y;

	// sends packet reply to client
	net.sendUdpPacket(snapshotPacket, "192.168.0.92", Properties::ClientPort);

	stateChanged = false;
}

// handles packets received - called as a callback from network object
void handlePacket(sf::Packet packet, sf::IpAddress sender, unsigned short port) {
	//using namespace std::this_thread;     // sleep_for, sleep_until
	//using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
	//sleep_for(500ms);
	int messageType; // message type
	long long clTimestamp; // client timestamp

	packet >> messageType >> clTimestamp;

	if (messageType == Message::Ping) {
		/*sf::Uint16 x = 1;*/
		//std::string response = "pong";
		using namespace std::chrono;
		// Get current time with precision of milliseconds
		auto now = time_point_cast<milliseconds>(system_clock::now());
		// sys_milliseconds is type time_point<system_clock, milliseconds>
		using sys_milliseconds = decltype(now);
		// Convert time_point to signed integral type
		auto svTimestamp = now.time_since_epoch().count();
		// Convert signed integral type to time_point
		sys_milliseconds time{ sys_milliseconds::duration{svTimestamp} };

		sf::Packet reply;
		reply << Message::Ping << clTimestamp << svTimestamp;

		// sends packet reply to client
		net.sendUdpPacket(reply, sender, Properties::ClientPort);
	}
	else {
		bool isPressed;
		packet >> isPressed;
		//std::cout << "Input: " << messageType << " : " << isPressed << std::endl;

		if (messageType == Message::PlayerMoveLeft) 
			left = isPressed;
		else if (messageType == Message::PlayerMoveRight)
			right = isPressed;
		else if (messageType == Message::PlayerMoveUp)
			up = isPressed;
		else if (messageType == Message::PlayerMoveDown)
			down = isPressed;

		stateChanged = true;
	}
}