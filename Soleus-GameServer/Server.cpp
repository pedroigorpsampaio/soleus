#include "Server.h"
#include <thread>
#include "Util.h"
#include "Creature.h"
#include <list>
#include "State.h"

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
sf::Vector2f lastPos(INIT_X, INIT_Y);
double snapshotTimer = 9999;
bool stateChanged = false;
sf::Vector2f velocity(0, 0);
size_t lastInput = 0;

// example creature
Creature demon(100, 100, 10, sf::Vector2f(INIT_X+50, INIT_Y+50));
// list of game active/alive creatures (temporary - to be later refactored when implementing AOI area of interest) 
std::list<Creature> creatures;


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

	// adds creature text to the list of alive creatures
	creatures.push_front(demon);

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

	// moves player if there is movement
	if (velocity.x != 0 || velocity.y != 0) {
		sf::Vector2f nVelocity = util::normalize(velocity);
		lastPos.x = playerPos.x; lastPos.y = playerPos.y; // stores player last pos
		playerPos.x += nVelocity.x * PLAYER_SPEED * dt; // not using dt because of sleep for ping simulation
		playerPos.y += nVelocity.y * PLAYER_SPEED * dt; // not using dt because of sleep for ping simulation
	}

	// moves example creature (test)
	demon.move(demon.getSpeed() * dt, 0.f);

	//std::cout << playerPos.x << std::endl;

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
	// updates server time
	using namespace std::chrono;
	// Get current time with precision of milliseconds
	auto now = time_point_cast<milliseconds>(system_clock::now());
	// sys_milliseconds is type time_point<system_clock, milliseconds>
	using sys_milliseconds = decltype(now);
	// Convert time_point to signed integral type
	auto svTimestamp = now.time_since_epoch().count();

	// sends only coords of the only example creature (demon) - to be refactored 
	// REFACTOR: Create a snapshot class that is packeable/unpackeable with all active entities
	// and items in aoi of player, alongside with player server data

	State state;
	state.nEntities = 1;
	state.nTiles = 0;
	//state.player = Entity();
	state.player.moveTo(playerPos.x, playerPos.y);
	state.entities.push_back(demon);

	/*snapshotPacket << Message::GameSync << playerPos.x << playerPos.y << lastInput << svTimestamp
		<< demon.getPos().x << demon.getPos().y;
*/

	snapshotPacket << Message::GameSync << state << lastInput << svTimestamp;

	//using namespace std::this_thread;     // sleep_for, sleep_until
	//using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
	//sleep_for(100ms);
	// sends packet reply to client
	net.sendUdpPacket(snapshotPacket, "192.168.0.92", Properties::ClientPort);

	stateChanged = false;
}

// handles packets received - called as a callback from network object
void handlePacket(sf::Packet packet, sf::IpAddress sender, unsigned short port) {
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
		//sys_milliseconds time{ sys_milliseconds::duration{svTimestamp} };

		sf::Packet reply;
		reply << Message::Ping << clTimestamp << svTimestamp;

		//using namespace std::this_thread;     // sleep_for, sleep_until
		//using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
		//sleep_for(100ms);
		// sends packet reply to client
		net.sendUdpPacket(reply, sender, Properties::ClientPort);
	}
	else {
		float vX, vY;
		size_t key;
		packet >> vX >> vY >> key;

		if (messageType == Message::PlayerMove) {
			velocity.x = vX; velocity.y = vY; // set player velocity values
			lastInput = key; // key of the last processed input (to be sent back to client for server reconciliation)
			// std::cout << velocity.x << " , " << velocity.y << std::endl;

			// if no movement, it is a stop moving command; send player one step back
			//if (vX == 0 && vY == 0) {
			//	playerPos.x = lastPos.x; playerPos.y = lastPos.y;
			//}
		}

		stateChanged = true;
	}
}