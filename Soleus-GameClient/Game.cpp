#include "Game.h"
#include "Util.h"
#include <list>    

float speed = PLAYER_SPEED; // moving speed
float zoomSpeed = 40.f; // zoom speed
sf::RenderWindow window; // game window
sf::Font font; 		// game main font

// game views
sf::View gameView, miniMapView, uiView;

// input handler
InputHandler inputHandler;

// game map texture
sf::RenderTexture texture;
// tilemap 
TileMap map;
// player
Player player(100, 100, 128, sf::Vector2f(INIT_X, INIT_Y));
// player client prediction and interpolation vars
sf::Vector2f initPos = player.getPos(), finalPos, deltaPos(0,0);
sf::Vector2f lastPos(INIT_X, INIT_Y);
float interpolIncX = 0, interpolIncY = 0;

struct PointInTime {
	sf::Vector2f pos;
	size_t key;
	long long timestamp;

	PointInTime(sf::Vector2f pos, size_t key, long long timestamp) {
		this->pos = pos; this->key = key; this->timestamp = timestamp;
	}
};

std::list<PointInTime> pitHistory;

// Network object that bridges communication with server
Networker net;

// client-server latency
long long latency = 0;
double pingTimer = 9999;
int pingInterval = 5; // latency check interval in seconds
long long svLastTimestamp = 0; // sv last timestamp
float lastDT = 0.16f; // sv last delta time
std::list<float> latencyHistory; // list of last latencies to calculate average latency
int latWindowSize = 5; // size of latency window for average calc

// fps text
sf::Text pingText, fpsText, svTimeText;

// mutex for multithreading data protection
//sf::Mutex mutex;

//// pinger thread - pings server every {interval}
//std::unique_ptr<sf::Thread> pingerThread;

Game *instance;
// constructor
Game::Game() {
	instance = this;
}

// loads game configurations - run at the start of game cycle
bool Game::load() {
	// create the window
	window.create(sf::VideoMode(960, 640), "Tilemap");
	// limits fps
	window.setVerticalSyncEnabled(true);
	// disable unwanted kep press event calls when key is being hold
	window.setKeyRepeatEnabled(false);
	// create the Input Handler for player inputs handling
	InputHandler inputHandler = InputHandler();

	// load game main font
	if (!font.loadFromFile("assets\\fonts\\arial.ttf")) {
		std::cout << "unable to load game main font" << std::endl;
	}

	// select the font
	pingText.setFont(font); // font is a sf::Font
	fpsText.setFont(font); // font is a sf::Font
	// set the string to display
	pingText.setString("Ping: 0ms");
	fpsText.setString("FPS: 60fps");

	// set the character size
	pingText.setCharacterSize(24); // in pixels, not points!
	fpsText.setCharacterSize(24); // in pixels, not points!

	// set the color
	pingText.setFillColor(sf::Color::White);
	fpsText.setFillColor(sf::Color::White);

	// set the text style
	pingText.setStyle(sf::Text::Bold);
	fpsText.setStyle(sf::Text::Bold);
	// set position
	fpsText.setPosition(0.0f, 30.0f);

	// select the font
	svTimeText.setFont(font); // font is a sf::Font
	// set the string to display
	svTimeText.setString("Client-Time: ");
	// set the character size
	svTimeText.setCharacterSize(10); // in pixels, not points!
	// set the color
	svTimeText.setFillColor(sf::Color::White);
	// set the text style
	svTimeText.setStyle(sf::Text::Bold);
	// set position
	svTimeText.setPosition(0.0f, 60.0f);


	// define the level with an array of tile indices
	const int level[] =
	{
		0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 0, 0, 2, 0, 3, 3, 3, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 0, 3, 3, 3, 0, 0, 0, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 1, 0, 3, 0, 2, 2, 0, 0, 1, 1, 1, 1, 2, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		2, 0, 1, 0, 3, 0, 2, 2, 2, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 1, 0, 3, 2, 2, 2, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	};

	// create the tilemap from the level definition
	if (!map.load("assets/tileset.png", sf::Vector2u(32, 32), level, 32, 23))
		std::cout << "error loading tileset" << std::endl;

	// create a view with the rectangular area of the 2D world to show
	gameView = sf::View (sf::FloatRect(32.f * 16, 23.f * 16, 300.f, 200.f));
	// viewport of game view
	gameView.setViewport(sf::FloatRect(0.0f, 0.0f, 0.85f, 1.0f));
	// initial zoom
	gameView.zoom(1.5f);
	// set game view
	window.setView(gameView);
	// minimap view
	miniMapView = sf::View(sf::FloatRect(32.f * 16, 23.f * 16, 480.f, 320.f));
	// viewport of minimap view
	miniMapView.setViewport(sf::FloatRect(0.85f, 0.0f, 0.15f, 0.15f));
	// ui view
	uiView = sf::View(sf::FloatRect(0, 0, 150.f, 320.f));
	// viewport of ui view
	uiView.setViewport(sf::FloatRect(0.85f, 0.15f, 0.15f, 0.85f));

	// Create a new render-texture to render tilemap
	if (!texture.create(960, 640))
		std::cout << "error creating map texture" << std::endl;

	texture.setSmooth(true);
	sf::Sprite sprite;

	// creates pinger thread - pings server every {pingerInterval}
	//pingerThread = std::make_unique<sf::Thread>(&Game::pinger, this);
	//// run it
	//pingerThread->launch();

	return true;
}

void Game::shutdown() {
	window.close();
}


// updates game before drawing
void Game::update(float dt) {
	// updates last delta time
	lastDT = dt;
	// obesrves packets received by server - sends the callback to handle them
	net.observe(handlePacket);

	// updates ping timer
	pingTimer += dt;
	long milli = dt * 1000;
	// updates game clock (aprox) with pingTimer (inbetween pings that adjusts to server-time)
	svLastTimestamp += milli;
	using namespace std::chrono;
	milliseconds dur(svLastTimestamp);
	time_point<system_clock> tpGameTime(dur);
	// updates svTime text
	std::stringstream svTimeStream;
	svTimeStream << getDateTime(tpGameTime) << std::endl;
	std::string svTimeStr = svTimeStream.str();
	svTimeText.setString(svTimeStr);

	// centers minimap based on game view
	miniMapView.setCenter(sf::Vector2f(gameView.getCenter()));

	// if reaches latency check interval, send ping msg to measure latency
	if (pingTimer >= pingInterval) {
		sendPingToServer();
		pingTimer = 0; // resets pingTimer for another round of pinging in a defined interval
	}

	// updates fps
	std::stringstream ss;
	int fps = 1 / dt;
	ss << "FPS: " << fps << "fps" << std::endl;
	std::string fpsStr = ss.str();
	fpsText.setString(fpsStr);

	// update event falgs
	sf::Event event;
	while (window.pollEvent(event)) {
		inputHandler.handleInput(event);

		// key events -- send to server for proper processing
		//if (event.type == sf::Event::KeyPressed || event.type == sf::Event::KeyReleased) {
		//	bool isPressed = event.type == sf::Event::KeyPressed ? true : false;
		//	if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S)
		//		sendPacketToServer(Message::PlayerMoveDown, isPressed);
		//	if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W)
		//		sendPacketToServer(Message::PlayerMoveUp, isPressed);
		//	if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::A)
		//		sendPacketToServer(Message::PlayerMoveLeft, isPressed);
		//	if (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::D)
		//		sendPacketToServer(Message::PlayerMoveRight, isPressed);
		//}

		int vX = 0, vY = 0;
		if (inputHandler.isKeyDown())
			vY = 1;
		if (inputHandler.isKeyUp())
			vY = -1;
		if (inputHandler.isKeyLeft())
			vX = -1;
		if (inputHandler.isKeyRight())
			vX += 1;
		// set player velocity
		player.setVelocity(vX, vY);

		// if player stopped moving, send move message (with no velocity) to stop player in server-side
		if (event.type == sf::Event::KeyReleased || event.type == sf::Event::KeyPressed) {
			if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S
				|| event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W
				|| event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::A
				|| event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::D) {
				sendPacketToServer(Message::PlayerMove);
			}
		}
	}

	// close games if close event is triggered
	if(inputHandler.isClosing())
		window.close();

	//// handle movement

	// input prediction
	sf::Vector2f nVelocity = util::normalize(player.getVelocity());
	//std::cout << nDirection.x << "," << nDirection.y << std::endl;
	//std::cout << player.getPos().x << std::endl;
	// if there is a movement
	if (nVelocity.x != 0 || nVelocity.y != 0) {
		// input prediction - move player
		player.move(nVelocity.x * PLAYER_SPEED * dt, nVelocity.y * PLAYER_SPEED * dt);

		//std::cout << player.getPos().x << std::endl;

		// saves new movement in front of list of input movements to proper conciliate with the server in sync moments
		pitHistory.push_front(PointInTime(player.getPos(), pitHistory.size()+1, svLastTimestamp));

		//std::cout << svLastTimestamp << std::endl;

		// sends packet to server to move player real position
		//sendPacketToServer(Message::PlayerMove);

		// only do smoothing while there is movement

		// smooth pos difference between server and client, if there is any
		//if (deltaPos.x != 0 || deltaPos.y != 0) {
		//	sf::Vector2f sPos(player.getPos().x + deltaPos.x, player.getPos().y + deltaPos.y);
		//	sf::Vector2f iPos = util::interpolate2v(player.getPos(), sPos);

		//	float distance = util::distance(player.getPos().x, player.getPos().y, sPos.x, sPos.y);

		//	if (distance > 0.1f && distance < 10.0f) {
		//		sf::Vector2f v = util::normalize(deltaPos);
		//		sf::Vector2f interpolatedPos = util::interpolate2v(player.getPos(), sf::Vector2f(sPos.x, sPos.y));
		//		player.moveTo(player.getPos().x + (interpolIncX * dt), player.getPos().y + interpolIncY * dt);
		//		player.moveTo(interpolatedPos.x, interpolatedPos.y);
		//		deltaPos.x = sPos.x - player.getPos().x;
		//		deltaPos.y = sPos.y - player.getPos().y;
		//	}
		//	else {
		//		sf::Vector2f interpolatedPos = util::interpolate2v(player.getPos(), sf::Vector2f(finalPos.x, finalPos.y));
		//		player.move(interpolIncX* dt, interpolIncY* dt);
		//		player.moveTo(interpolatedPos.x, interpolatedPos.y); 
		//		deltaPos.x = 0; deltaPos.y = 0; /*pitHistory.clear();*/
		//	}

			//distance = util::distance(player.getPos().x, player.getPos().y, finalPos.x, finalPos.y);

			//if (distance > 0.25f) {
			//	sf::Vector2f interpolatedPos = util::interpolate2v(player.getPos(), sf::Vector2f(finalPos.x, finalPos.y));
			//	player.moveTo(interpolatedPos.x, interpolatedPos.y);
			//}
			//else { player.moveTo(finalPos.x, finalPos.y); }

			//std::cout << distance << "," << deltaPos.y << std::endl;
		//}

		//float svDiffX = player.getPos().x - finalPos.x;
		//std::cout << "sv diff: " << svDiffX << std::endl;
	}

	// (FINAL SERVER POSITION) move by interpolated amount - to be used for other entities
	//float distance = util::distance(player.getPos().x, player.getPos().y, finalPos.x, finalPos.y);

	//if (distance > 0.25f) {
	//	if (player.getPos().x != finalPos.x)
	//		player.move(interpolIncX * dt, 0.f);
	//	if (player.getPos().y != finalPos.y)
	//		player.move(0.f, interpolIncY * dt);
	//}
	//else { player.moveTo(finalPos.x, finalPos.y); }

	// (PING INCLUDED DIFF SERVER POSITION) move by interpolated amount
	float distance = util::distance(player.getPos().x, player.getPos().y, lastPos.x, lastPos.y);

	if (distance > 0.5f) {
		if (player.getPos().x != lastPos.x)
			player.move(interpolIncX * dt, 0.f);
		if (player.getPos().y != lastPos.y)
			player.move(0.f, interpolIncY * dt);
	}
	else { player.moveTo(lastPos.x, lastPos.y); }

	//float svDiffX = player.getPos().x - lastPos.x;
	//std::cout << "sv diff: " << svDiffX << std::endl;

	// centers map based on player 
	sf::Vector2f centeredPlayerPos(player.getPos().x + player.getCenterOffset().x,
									player.getPos().y + player.getCenterOffset().y);
	gameView.setCenter(sf::Vector2f(centeredPlayerPos));

	// handle zooming
	if (inputHandler.isZooming()) {
		if (inputHandler.getZoomDelta() == 1)
			gameView.zoom(1 - 0.1f * zoomSpeed * dt);
		else
			gameView.zoom(1 + 0.1f * zoomSpeed * dt);
		// halt all events to stop wheel zooming
		inputHandler.halt();
	}

}

// draws game - parte of gameloop
void Game::draw()
{
	// Clear the whole texture with red color
	texture.clear(sf::Color::Red);
	// Draw stuff to the texture
	texture.draw(map);

	// We're done drawing to the texture
	texture.display();

	// player sprites and minimap pinpoint
	sf::Sprite playerExample = player.getSprite();
	sf::CircleShape playerPoint;
	playerExample.setPosition(player.getPos().x - player.getCenterOffset().x, player.getPos().y - player.getCenterOffset().y);
	playerPoint.setRadius(6);
	playerPoint.setFillColor(sf::Color::White);
	playerPoint.setPosition(gameView.getCenter().x - 6, gameView.getCenter().y - 6);

	// clear the window to start rendering it
	window.clear();
	// draw game view
	window.setView(gameView);
	// Draw the texture for game view
	sf::Sprite sprite(texture.getTexture());
	window.draw(sprite);
	window.draw(playerExample);
	//window.draw(map);
	// draw texture for minimap view
	window.setView(miniMapView);
	window.draw(sprite);
	window.draw(playerPoint);
	//window.draw(map);
	// draw in ui view
	window.setView(uiView);
	window.draw(pingText);
	window.draw(fpsText);
	window.draw(svTimeText);
	// end current window frame and displays its content
	window.display();
}

/// Prepares the different types of packets and send to server 
void Game::sendPacketToServer(int messageType) {
	sf::Packet packet;
	packet << messageType;

	if (messageType == Message::Ping) {
		sendPingToServer();
		return;
	}
	else {
		if (messageType == Message::PlayerMove) {
			// gets input key (for server reconciliation of client prediction later)
			size_t key = 0;
			//key = pitHistory.front().key;
			if (pitHistory.size() > 0) {
				for (PointInTime pH : pitHistory) {
					key = pH.key;
					break;
				}
			}
			// prepares packet of input type
			packet << svLastTimestamp << player.getVelocity().x  << player.getVelocity().y << key;
			// sends packet
			net.sendUdpPacket(packet, Properties::ServerIP, Properties::ServerPort);
		}
			/*std::cout << "Unknown message type - Message not sent to server" << std::endl;*/
	}
}

// sends ping packet to game server
void Game::sendPingToServer() {
	using namespace std::chrono;
	// Get current time with precision of milliseconds
	auto now = time_point_cast<milliseconds>(system_clock::now());
	// sys_milliseconds is type time_point<system_clock, milliseconds>
	using sys_milliseconds = decltype(now);
	// Convert time_point to signed integral type
	auto timestamp = now.time_since_epoch().count();
	// Convert signed integral type to time_point
	//sys_milliseconds time{ sys_milliseconds::duration{timestamp} };

	// placehold ping packet
	sf::Packet packet;
	packet << Message::Ping << timestamp;
	// sends packet via send udp method
	net.sendUdpPacket(packet, Properties::ServerIP, Properties::ServerPort);
}

// handles packets received - called as a callback from network object
void handlePacket(sf::Packet packet, sf::IpAddress sender, unsigned short port) {
	int messageType;

	packet >> messageType;

	if (messageType == Message::Ping) {
		syncClock(packet);
	} 
	else if (messageType == Message::GameSync){
		//instance->sendPacketToServer(Message::PlayerMove);
		float x, y;
		size_t lastInput; 
		long long svTimestamp; // sv timestamp of when packet was sent
		packet >> x >> y >> lastInput >> svTimestamp;
		
		// approximation of timestamp when last input was processed
		//long long lastTime = svLastTimestamp - (latency/2);
		//std::cout << "why im here with lag??? high ping??? wtf :: " << svTimestamp << std::endl;
		// if found the last action to sync drift
		bool found = false;
		// finds correct input to calculate sv delta pos
		if (pitHistory.size() > 0) {
			for (PointInTime n : pitHistory) {
				//std::cout << lastInput << " / " << n.key << " -> " << n.pos.x << " , " << x << " | tk: " << n.timestamp << " : ts: " << svTimestamp << std::endl;
				if (n.timestamp < svTimestamp) {
					found = true;
					//std::cout << "dif delta : " << player.getPos().x - n.pos.x << std::endl;
					lastPos = n.pos;
					
					//std::cout << lastInput << " / " << n.key << " -> " << n.pos.x << " , " << x << " | tk: " << n.timestamp << " : ts: " << lastTime << std::endl;
					break;
					//std::cout << lastInput << " -> " << player.getPos().x << " , " << x << std::endl;
				}
			}
			if (!found) // if not found a prior action from client to match server time of packet, get the oldest one
				lastPos = pitHistory.back().pos;

			pitHistory.clear(); // when sync action is found, clear history and use the found position to calculate drift 
			float difX = x - lastPos.x;

			//std::cout << difX << std::endl;
			//std::cout << std::endl;
		}

		//std::cout << svTimestamp << " / " << lastTime << std::endl;

		// drift calculation for
		// server reconciliation
		float difX = x - lastPos.x;

		//std::cout << difX << std::endl;

		float difY = y - lastPos.y;
		// interpolation to be used later - example (used with commented block in update)
		finalPos.x = x; finalPos.y = y;
		interpolIncX = ((difX) / (60.0f / SNAPSHOT_TICKRATE)) ;
		interpolIncY = ((difY) / (60.0f / SNAPSHOT_TICKRATE)) ;
		if (difX != 0 || difY != 0) {
			//float distance = util::distance(player.getPos().x, player.getPos().y, x, y);

			//if (distance > 0.25f) {
			//	sf::Vector2f interpolatedPos = util::interpolate2v(player.getPos(), sf::Vector2f(x, y));
			//	player.moveTo(interpolatedPos.x, interpolatedPos.y);
			//}
			//else { player.moveTo(x, y); }
		}

		deltaPos.x = difX;
		deltaPos.y = difY;

		// Print out the list
		if (pitHistory.size() > 0) {
			//std::cout << "sv_late: " << x << "," << y << std::endl;

			//std::cout << "l = { ";
			//for (PointInTime n : pitHistory)
			//	std::cout << "(" << n.pos.x << "," << n.pos.y << ")" << ", ";
			//std::cout << "};\n";

			// clears list of point in time inputs to restart input storage
			/*pitHistory.clear();*/
		}
	}
}

/// sync client clock with server clock
void syncClock(sf::Packet packet) {
	//using namespace std::this_thread;     // sleep_for, sleep_until
	//using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
	//sleep_for(1000ms);

	long long clTimestamp, svTimestamp;
	packet >> clTimestamp >> svTimestamp;

	using namespace std::chrono;
	// Get current time with precision of milliseconds
	auto now = time_point_cast<milliseconds>(system_clock::now());
	// sys_milliseconds is type time_point<system_clock, milliseconds>
	using sys_milliseconds = decltype(now);
	// Convert time_point to signed integral type
	auto timestampNow = now.time_since_epoch().count();

	// calculates the elapsed time since packet was sent from server
	// to adjust client clock to match the server
	latency = (timestampNow - clTimestamp); // round trip
	auto halfRT = latency / 2;

	// synchronized timestamp
	auto syncTimestamp = svTimestamp + halfRT;
	svLastTimestamp = syncTimestamp;

	//// Convert signed integral type to time_point
	//sys_milliseconds syncTime{ sys_milliseconds::duration{syncTimestamp} };

	// updates latency history for the defined window size
	latencyHistory.push_front(latency);
	// pops oldest latency data if surpasses window size
	if (latencyHistory.size() > latWindowSize)
		latencyHistory.pop_back();

	// calculate average of latency in defined window size
	int avgLat = 0, acc = 0;
	for (int l : latencyHistory) {
		acc += l;
	}
	avgLat = acc / latencyHistory.size();

	std::stringstream ss;
	ss << "Ping: " << avgLat << "ms" << std::endl;
	std::string pingStr = ss.str();
	pingText.setString(pingStr);
}
