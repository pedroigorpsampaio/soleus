#ifndef MESSENGER_H
#define MESSENGER_H

#include <SFML/Network.hpp>

namespace Message
{
	/// Values that represent the different Packet Types.
	enum {
		//---------------------------------------------
		// Type                      Additional data sent 
		//---------------------------------------------
		ClientConnected,    ///< (int)playerIndex
		GameStarted,
		PlayerClick, /// player input of click type with position data and timestamp
		PlayerMove,  /// player input of the move type with velocity data and timestamp
		Ping, /// ping message with timestamps to calculate latency
		GameSync /// sync message containing information about other entities and items
	};
}

// network properties
namespace Properties {
	const sf::IpAddress ServerIP = "192.168.0.92";
	const unsigned short ServerPort = 43570;
	const unsigned short ClientPort = 43579;
}

// An abstract class that controls the communication between messengers
class Messenger {
	public:
		// Observes packets received in each cycle and alerts callback (argument) when a packet is received
		virtual void observe(void (*handlePacket)(sf::Packet, sf::IpAddress, unsigned short)) = 0;
		// Sends udp packets to a recipient
		virtual int sendUdpPacket(sf::Packet packet, sf::IpAddress recipient, unsigned short port) = 0;
	protected:
		// Bind the socket
		virtual int bindSocket() = 0;
		// Receives udp packets and alerts callback
		virtual void receiveUdpPacket(void(*handlePacket)(sf::Packet, sf::IpAddress, unsigned short)) = 0;
};

#endif

