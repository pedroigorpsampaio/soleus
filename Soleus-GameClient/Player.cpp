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
	collider.updateCollider(getPos());
}

void Player::onCollisionEnter(tmx::Object source) {
	if (source.getName().compare("hole_down") == 0 || source.getName().compare("stair_down_downward") == 0) {
		if (!changingFloors) { // do not change floors if it is currently changing floors
			floor = floor - 1;
			changingFloors = true;
			// move player according to hole/stairs position
			// must be done in server also, and has to be done in a way that player dont
			// walk back up unintentionally

			// TODO NOW !!!!
			// STAIRS UPWARD!!!

			//move(TILESIZE + (TILESIZE * velocity.x), TILESIZE + (TILESIZE * velocity.y));
			moveTo(source.getAABB().left + TILESIZE * 1.04f, source.getAABB().top + TILESIZE * 0.825f);
			//move(TILESIZE * 0.75f * velocity.x, TILESIZE * 0.5f * velocity.y);
			//move(TILESIZE + (source.getAABB().width * velocity.x), TILESIZE*0.81f + ((source.getAABB().height/2) * velocity.y));
		}
		else { changingFloors = false; } // reset floor change flag
	}
	else if (source.getName().compare("stair_up_downward") == 0 || source.getName().compare("ladder_up") == 0) {
		if (!changingFloors) { // do not change floors if it is currently changing floors
			floor = floor + 1;
			changingFloors = true;
			// move entity(player) according to hole/stairs position
			// must be done in server also, and has to be done in a way that player dont
			// walk back down unintentionally

			//move(TILESIZE * velocity.x, TILESIZE * velocity.y);
			moveTo(source.getAABB().left - TILESIZE * 0.9f, source.getAABB().top - TILESIZE * 1.1f);
			//move(TILESIZE*0.75f * velocity.x, TILESIZE * 0.75f * velocity.y);
		}
		else { changingFloors = false; } // reset floor change flag
	}
}

void Player::onCollisionStay(tmx::Object source) {
}

void Player::onCollisionExit(tmx::Object source) {
}
