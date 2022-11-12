#include "Player.h"
#include <iostream>

// gets player current lowest visible floor as an array index
int Player::getLowestVisibleFloor() {
	int initI = (floor - N_FLOOR_DOWN_VISIBLE) + abs(MIN_FLOOR); // calculates min floor
	if (initI < 0) initI = 0; // player is already on min floor, dont show floor below because it does not exist
	return initI;
}

// gets player current highest visible floor as an array index
/// arraySize - the array size of the stored layers - to not surpass it
int Player::getHighestVisibleFloor(int arraySize) {
	int finalI = (floor + N_FLOOR_UP_VISIBLE) + abs(MIN_FLOOR); // calculates max floor
	if (finalI >= arraySize) finalI = arraySize - 1; // safe - and to not show a floor above the highest one
	return finalI;
}

// updates player
void Player::update(float dt) {
	// updates player collider
	updateCollider();
}