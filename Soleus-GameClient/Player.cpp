#include "Player.h"
#include <iostream>

int Player::load() {
	// loads player texture
	
	if (!texture.loadFromFile("assets/sprites/player_tst.png")) {
		std::cout << "Could not load player texture" << std::endl;
		return -1;
	}

	texture.setSmooth(true);
	sprite.setTexture(texture);
	sprite.scale(sf::Vector2f(0.2, 0.2));

	return 0;
}
