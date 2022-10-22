#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <chrono>
#include <thread>

sf::UdpSocket socket;
unsigned short gameServerPort = 43570;
unsigned short clientPort = 43579;


int main()
{
	sf::RenderWindow window(sf::VideoMode(200, 200), "SFML works!");
	sf::CircleShape shape(100.f);
	shape.setFillColor(sf::Color::Green);

	//sf::Uint16 x;
	std::string s;
	//double d = 5;

	socket.setBlocking(false);

	// bind the socket to a port
	if (socket.bind(gameServerPort) != sf::Socket::Done)
	{
		// error...
	}

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear();
		window.draw(shape);
		window.display();

		sf::IpAddress sender;
		unsigned short port;
		sf::Packet packet;

		if (socket.receive(packet, sender, port) != sf::Socket::Done) {
			// no msgs being received 
		}
		else { // msg received			

			using namespace std::this_thread;     // sleep_for, sleep_until
			using namespace std::chrono_literals; // ns, us, ms, s, h, etc.

			sleep_for(100ms);

			packet >> s;

			
			if (s.compare("ping") == 0) {
				/*sf::Uint16 x = 1;*/
				std::string response = "pong";

				sf::Packet reply;
				reply << response;

				//std::cout << sender << ":" << clientPort;

				// sends packet reply
				if (socket.send(reply, sender, clientPort) != sf::Socket::Done) {
					// error...
					std::cout << "Error sending packet to client";
					return -1;
				}
			}
		}

		//std::cout << d;
	}

	return 0;
}