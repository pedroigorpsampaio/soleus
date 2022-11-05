#include "Networker.h"
#include <iostream>

// binds socket to the desired port
int Networker::bindSocket() {
	socket.setBlocking(false); // non-binding socket
	// binds to desired port
	if (socket.bind(Properties::ClientPort) != sf::Socket::Done) {
		// error...
		std::cout << "Error binding socket to desired port";
		return -1;
	}

	return 0;
}

// check if a packet is received
void Networker::observe(void(*handlePacket)(sf::Packet, sf::IpAddress, unsigned short)){
	receiveUdpPacket(handlePacket); // checks each cycle if a udp packet is received and calls callback if so
}

// sends packet to the desired recipient
int Networker::sendUdpPacket(sf::Packet packet, sf::IpAddress recipient, unsigned short port) {
	// sends packet
	if (socket.send(packet, recipient, port) != sf::Socket::Done) {
		// error...
		std::cout << "Error sending packet to game server";
		return -1;
	}

	return 0;
}

// Receieves udp packets and alerts callback
void Networker::receiveUdpPacket(void(*handlePacket)(sf::Packet, sf::IpAddress, unsigned short)) {
	sf::IpAddress sender;
	unsigned short port;
	sf::Packet packet;

	// receives packets
	if (socket.receive(packet, sender, port) != sf::Socket::Done) {
		// no packet being received 
	}
	else { // packet received
		handlePacket(packet, sender, port); // calls the callback received as an argument passing the packet and sender data
	}
}

//template<typename T>
//sf::Packet& operator<<(sf::Packet& packet, std::vector<T>& vec)
//{
//	return packet << vec.at(0) << vec.at(1) << vec.at(2);
//}
//
//template<typename T>
//sf::Packet& operator>>(sf::Packet& packet, std::vector<T>& vec)
//{
//	return packet >> vec.at(0) >> vec.at(1) >> vec.at(2);
//}

//////////////// PACKING AND UNPACKING ///////////////////

/// packs entity into packet
sf::Packet& operator<<(sf::Packet& packet, Entity& entity)
{
	return packet << entity.health << entity.maxHealth << entity.pos.x << entity.pos.y;
}

/// unpacks entity from packet
sf::Packet& operator>>(sf::Packet& packet, Entity& entity)
{
	return packet >> entity.health >> entity.maxHealth >> entity.pos.x >> entity.pos.y;
}

/// packs state into packet
sf::Packet& operator<<(sf::Packet& packet, State& state)
{
	// player
	packet << state.player;

	// sizes of arrays packed
	packet << state.nTiles << state.nEntities;

	// tiles visible for player
	for (int i = 0; i < state.nTiles; i++)
		packet << state.tiles.at(i);

	// entities visible for player
	for(int i = 0; i < state.nEntities; i++)
		packet << state.entities.at(i);

	return packet;
}

/// unpacks state from packet
sf::Packet& operator>>(sf::Packet& packet, State& state)
{
	// player
	packet >> state.player;

	// sizes of arrays packed
	packet >> state.nTiles >> state.nEntities;

	// tiles visible for player
	for (int i = 0; i < state.nTiles; i++) {
		int t;
		packet >> t;
		state.tiles.push_back(t);
	}

	// entities visible for player
	for (int i = 0; i < state.nEntities; i++) {
		Entity e;
		packet >> e;
		state.entities.push_back(e);
	}

	return packet;
}