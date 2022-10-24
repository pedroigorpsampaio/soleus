#ifndef NETWORKER_H
#define NETWORKER_H

#include "Messenger.h"

class Networker : public Messenger {
public:
	Networker() { bindSocket(); } // constructor - binds socket
	// Observes packets received in each cycle and alerts callback (argument) when a packet is received
	virtual void observe(void (*handlePacket)(sf::Packet, sf::IpAddress, unsigned short)) override;
	// Sends udp packets to a recipient
	virtual int sendUdpPacket(sf::Packet packet, sf::IpAddress recipient, unsigned short port) override;

protected:
	// the socket used for communication
	sf::UdpSocket socket;
	// Bind the socket
	virtual int bindSocket() override;
	// Receieves udp packets and alerts callback
	virtual void receiveUdpPacket(void(*handlePacket)(sf::Packet, sf::IpAddress, unsigned short)) override;
};

#endif