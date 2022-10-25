#include "Game.h"
#include "Util.h"

float speed = 100.f; // moving speed
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
Player player(100, 100, 128, sf::Vector2f(0, 0));

// Network object that bridges communication with server
Networker net;

// client-server latency
long long latency = 0;
double pingTimer = 9999;
int pingInterval = 5; // latency check interval in seconds
long long svLastTimestamp = 0; // sv last timestamp


// fps text
sf::Text pingText, fpsText, svTimeText;

// mutex for multithreading data protection
//sf::Mutex mutex;

//// pinger thread - pings server every {interval}
//std::unique_ptr<sf::Thread> pingerThread;

// constructor
Game::Game() {}

// loads game configurations - run at the start of game cycle
bool Game::load() {
	// create the window
	window.create(sf::VideoMode(960, 640), "Tilemap");
	// limits fps
	window.setVerticalSyncEnabled(true);
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
	
	//using namespace std::chrono;
	//gameClock += (dt*1000);
	//std::cout << gameClock << std::endl;

	//using sys_milliseconds = decltype(time_point_cast<milliseconds>(system_clock::now()));
	//// Convert signed integral type to time_point
	//sys_milliseconds gameTime{ sys_milliseconds::duration{gameClock} };
	//// updates svTime text
	//std::stringstream svTimeStream;
	//svTimeStream << getDateTime(gameTime) << std::endl;
	//std::string svTimeStr = svTimeStream.str();
	//svTimeText.setString(svTimeStr);

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
	}

	// close games if close event is triggered
	if(inputHandler.isClosing())
		window.close();

	// handle movement
	if (inputHandler.isKeyDown())
		gameView.move(0.f, speed * dt);
	if (inputHandler.isKeyUp()) {
		/*sf::Uint16 x = 10;
		std::string s = "latency";
		double d = 0.6;

		
		sf::Packet packet;
		packet << x << s << d;

		startLatencyTimer = std::chrono::system_clock::now();

		sendUdpPacket(packet, gameServerIP, gameServerPort);*/
		
		//sf::Packet reply = receiveUdpPacket(view, dt);
		gameView.move(0.f, -speed * dt);
	}
	if (inputHandler.isKeyLeft())
		gameView.move(-speed * dt, 0.f);
	if (inputHandler.isKeyRight())
		gameView.move(speed * dt, 0.f);

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
	playerExample.setPosition(gameView.getCenter().x - 16, gameView.getCenter().y - 16);
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
	sys_milliseconds time{ sys_milliseconds::duration{timestamp} };

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

	std::stringstream ss;
	ss << "Ping: " << latency << "ms" << std::endl;
	std::string pingStr = ss.str();
	pingText.setString(pingStr);
}