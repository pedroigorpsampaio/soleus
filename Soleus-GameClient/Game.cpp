#include "Game.h"

// timers
std::chrono::time_point<std::chrono::system_clock> startLatencyTimer = std::chrono::system_clock::now();
std::chrono::time_point<std::chrono::system_clock> endLatencyTimer = std::chrono::system_clock::now();

float speed = 100.f; // moving speed
float zoomSpeed = 400.f; // zoom speed
sf::RenderWindow window; // game window
sf::Clock deltaClock;	// game delta clock
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

// game server properties
sf::IpAddress gameServerIP = "192.168.0.92";
unsigned short gameServerPort = 43570;
unsigned short clientPort = 43579;
sf::UdpSocket gameServerSocket;
sf::UdpSocket clientSocket;

// client-server latency
long latency = 0;
sf::Text pingText;
double pingTimer = 0;
int pingInterval = 1; // latency check interval in seconds

// constructor just calls load method that loads configuration
Game::Game() {
	load();
}

// loads game configurations - run at the start of game cycle
void Game::load() {
	// loads connection
	loadConnection();
	// create the window
	window.create(sf::VideoMode(960, 640), "Tilemap");
	// create the Input Handler for player inputs handling
	InputHandler inputHandler = InputHandler();

	// load game main font
	if (!font.loadFromFile("assets\\fonts\\arial.ttf")) {
		std::cout << "unable to load game main font" << std::endl;
	}

	// select the font
	pingText.setFont(font); // font is a sf::Font

	// set the string to display
	pingText.setString("Ping: 0ms");

	// set the character size
	pingText.setCharacterSize(24); // in pixels, not points!

	// set the color
	pingText.setFillColor(sf::Color::White);

	// set the text style
	pingText.setStyle(sf::Text::Bold);

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
}

// runes gameloop - update and draw
void Game::run() {
	// creates player
	// player = Player(100, 100, 128, sf::Vector2f(0, 0));

	// run the main loop
	while (window.isOpen())
	{
		// updates game clock
		sf::Time dt = deltaClock.restart();

		// updates latency timer
		pingTimer += dt.asSeconds();

		// updates game
		update(dt.asSeconds());

		// draws game
		draw();
	}

}

// updates game before drawing
void Game::update(float dt) {
	miniMapView.setCenter(sf::Vector2f(gameView.getCenter()));

	// if reaches latency check interval, send ping msg to measure latency
	if (pingTimer >= pingInterval)
		sendPingToServer();

	// check packets received via udp
	receiveUdpPacket(dt);

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
	// end current window frame and displays its content
	window.display();
}

// loads connection properties
int Game::loadConnection() {
	clientSocket.setBlocking(false);
	// bind the client socket to a port
	if (clientSocket.bind(clientPort) != sf::Socket::Done) {
		// error...
		std::cout << "Error binding client socket to desired port";
		return -1;
	}

	return 0;
}

// sends packet to a server using UDP Protocol
int Game::sendUdpPacket(sf::Packet packet, sf::IpAddress recipient, unsigned short port) {

	// sends packet
	if (gameServerSocket.send(packet, recipient, port) != sf::Socket::Done) {
		// error...
		std::cout << "Error sending packet to game server";
		return -1;
	}

	return 0;
}

// receives udp packet
void Game::receiveUdpPacket(float dt) {
	sf::IpAddress sender;
	unsigned short port;
	sf::Packet packet;

	// receives packets
	if (clientSocket.receive(packet, sender, port) != sf::Socket::Done) {
		// no msgs being received 
	}
	else { // msg received
		std::string s;

		packet >> s;

		if (s.compare("pong") == 0) {
			endLatencyTimer = std::chrono::system_clock::now();

			sf::Uint32 timestamp = std::chrono::duration_cast<std::chrono::milliseconds>
				(std::chrono::time_point_cast<std::chrono::milliseconds>
				(std::chrono::high_resolution_clock::now()).time_since_epoch()).count();

			std::chrono::duration<double> elapsed_seconds = endLatencyTimer - startLatencyTimer;
			auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_seconds).count();

			
			latency = millis;
			//std::cout << "Ping: " << latency << "ms" << std::endl;
			std::stringstream ss;
			ss << "Ping: " << latency << "ms" << std::endl;
			std::string pingStr = ss.str();
			pingText.setString(pingStr);
		}
	}
}

// sends ping packet to game server
void Game::sendPingToServer() {
	pingTimer = 0; // resets pingTimer for another round of pinging in a defined interval
	// placehold ping packet
	sf::Packet packet;
	packet << "ping";
	// sends packet via send udp method
	sendUdpPacket(packet, gameServerIP, gameServerPort);
	// updates timer to measure latency
	startLatencyTimer = std::chrono::system_clock::now();
}
