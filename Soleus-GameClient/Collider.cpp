#include "Collider.h"
#include "Util.h"

Collider::Collider() {
	this->trigger = false; this->owner = 0;
}

void Collider::registerOwner(Collidable *owner) {
	this->owner = owner;
}

void Collider::setup(float left, float top, float width, float height, bool trigger) {
	rect.left = left; rect.top = top; rect.width = width; rect.height = height;
	this->trigger = trigger;
}

sf::Rect<float> Collider::getRect() {
	return rect;
}

void Collider::setRect(float left, float top, float width, float height) {
	rect.left = left; rect.top = top; rect.width = width; rect.height = height;
}

bool Collider::isTrigger() {
	return trigger;
}

void Collider::setTrigger(bool isTrigger) {
	trigger = isTrigger;
}

// checks collision with objects (trigger collision) 
void Collider::checkCollision(tmx::Object object) {
	bool col = util::checkRectCollision(rect.left, rect.top, rect.width, rect.height,
		object.getAABB().left, object.getAABB().top, object.getAABB().width, object.getAABB().height);
	//std::cout << col << std::endl;
	if (col) { // if collision happens
		// checks if it is already on list of collisions
		bool isColliding = false;
		for (const auto& o : collisionObjects) {
			if (o.getUID() == object.getUID()) { // check by uniqueID
				isColliding = true;
			}
		}
		if (!isColliding) { // if not colliding yet, add to list of object collisions and trigger on enter collision
			collisionObjects.push_back(object);
			owner->onCollisionEnter(object);
		}
		else { // if is already colliding, trigger on stay collision
			owner->onCollisionStay(object);
		}
	}
}

// checks collision with objects (trigger collision) taking into account a movement to be added
void Collider::checkCollision(tmx::Object object, sf::Vector2f move) {
	bool col = util::checkRectCollision(rect.left + move.x, rect.top + move.y, rect.width, rect.height,
		object.getAABB().left, object.getAABB().top, object.getAABB().width, object.getAABB().height);
	//std::cout << col << std::endl;
	if (col) { // if collision happens
		// checks if it is already on list of collisions
		bool isColliding = false;
		for (const auto& o : collisionObjects) {
			if (o.getUID() == object.getUID()) { // check by uniqueID
				isColliding = true;
			}
		}
		if (!isColliding) { // if not colliding yet, add to list of object collisions and trigger on enter collision
			collisionObjects.push_back(object);
			owner->onCollisionEnter(object);
		}
		else { // if is already colliding, trigger on stay collision
			owner->onCollisionStay(object);
		}
	}
}

// checks physical collision (collisions that block movement)
bool Collider::checkPhysCollision(tmx::Object object) {
	bool col = util::checkRectCollision(rect.left, rect.top, rect.width, rect.height,
		object.getAABB().left, object.getAABB().top, object.getAABB().width, object.getAABB().height);
	return col;
}

// checks physical collision (collisions that block movement) taking into account a movement to be added
bool Collider::checkPhysCollision(tmx::Object object, sf::Vector2f move) {
	bool col = util::checkRectCollision(rect.left + move.x, rect.top + move.y, rect.width, rect.height,
		object.getAABB().left, object.getAABB().top, object.getAABB().width, object.getAABB().height);
	return col;
}

// sets default collider offsets
void Collider::adjustCollider() {
	colOffset.left = 0; colOffset.top = 0;
	colOffset.width = 100;
	colOffset.height = 100;
}

// sets collider offsets and dimensions
void Collider::adjustCollider(float offX, float offY, float width, float height) {
	colOffset.left = offX; colOffset.top = offY;
	colOffset.width = width;
	colOffset.height = height;
}


// updates collider with owner position
void Collider::updateCollider(sf::Vector2f pos) {
	// updates collider position based on owner pos
	setRect(pos.x + colOffset.left, pos.y + colOffset.top, colOffset.width, colOffset.height);

	// updates list of map collisions removing from it when collision is not happening anymore
	for (int i = collisionObjects.size() - 1; i >= 0; i--) {
		const auto& o = collisionObjects.at(i);
		bool col = util::checkRectCollision(rect.left, rect.top,
			rect.width, rect.height,
			o.getAABB().left, o.getAABB().top, o.getAABB().width, o.getAABB().height);
		if (!col) { // if collision does not happen
			collisionObjects.erase(collisionObjects.begin() + i); // remove from list os collisions
			owner->onCollisionExit(o); // calls on collision exit callback
		}
		else { // if collision still happens, call on collision stay
			owner->onCollisionStay(o);
		}
	}
}