#include "Entity.h"
#include <iostream>

float incX, incY; // interpolation increments
float fps; // current game fps

Entity::Entity() {
	health = 100; maxHealth = 100; speed = 100; pos = sf::Vector2f(0, 0); velocity = sf::Vector2f(0, 0);
	active = false; floor = INIT_FLOOR;
}

Entity::Entity(int health, int maxHealth, int speed, sf::Vector2f initialPos):
	health(health), maxHealth(maxHealth), speed(speed), pos(initialPos)
{
	velocity = sf::Vector2f(0, 0);
	active = false; floor = INIT_FLOOR;
}

int Entity::getHealth()
{
	return health;
}

int Entity::getMaxHealth()
{
	return maxHealth;
}

int Entity::getSpeed()
{
	return speed;
}

sf::Vector2f Entity::getPos()
{
	return pos;
}

sf::Vector2f Entity::getVelocity()
{
	return this->velocity;
}

sf::Vector2f Entity::getCenterOffset()
{
	int offX = this->getSprite().getTexture()->getSize().x * this->getSprite().getScale().x / 2;
	int offY = this->getSprite().getTexture()->getSize().y * this->getSprite().getScale().y / 2;

	return 	sf::Vector2f(offX, offY);
}

sf::Sprite Entity::getSprite()
{
	return sprite;
}

void Entity::setVelocity(sf::Vector2f velocity)
{
	this->velocity = velocity;
}

void Entity::setVelocity(float vX, float vY)
{
	this->velocity.x = vX;
	this->velocity.y = vY;
}

void Entity::move(sf::Vector2f offset)
{
	this->pos + offset;
}

void Entity::move(float offsetX, float offsetY)
{
	this->pos.x += offsetX; this->pos.y += offsetY;
}

void Entity::moveTo(float x, float y) 
{
		this->pos.x = x;
		this->pos.y = y;
}

void Entity::interpolate(float x, float y) {
	this->goal = sf::Vector2f(x, y);
	incX = (x - pos.x) / (fps / SNAPSHOT_TICKRATE);
	incY = (y - pos.y) / (fps / SNAPSHOT_TICKRATE);
}

void Entity::interpolate(sf::Vector2f goal) {
	this->goal = goal;
	incX = (goal.x - pos.x) / (fps / SNAPSHOT_TICKRATE);
	incY = (goal.y - pos.y) / (fps / SNAPSHOT_TICKRATE);
}

int Entity::react(Entity source, Event event)
{
	return 0;
}

void Entity::checkCollision(tmx::Object object) {
}

void Entity::checkCollision(tmx::Object object, sf::Vector2f move) {
	bool col = util::checkRectCollision(collider.left + move.x, collider.top + move.y, collider.width, collider.height,
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
			// only calls onCollision enter when not changing floors 
			// because new floor stair collider should not be triggered 
			// while changing floors
			if (!changingFloors)
				onCollisionEnter(object);
			else
				changingFloors = false; // reset changing floor flag
		}
		else { // if is already colliding, trigger on stay collision
			onCollisionStay(object);
		}
	}
}

void Entity::onCollisionEnter(tmx::Object source) {
	if (source.getName().compare("hole_down") == 0 || source.getName().compare("stair_down") == 0) {
		floor = floor - 1;
		changingFloors = true;
		// move entity(player) according to hole/stairs position
		// must be done in server also, and has to be done in a way that player dont
		// walk back up unintentionally

		// TODO NOW !!!!
		// IMPLEMENT ON TRIGGER STAY, LEAVE AND ENTER <-> BETTER COLLIDER ARCHITECTURE
		// TO BE ABLE TO REACT TO STAY, LEAVE AND ENTER COLLISIONS - 
		// ONLY CHANGE FLOORS WHEN ENTERING COLLISION 
		// TO BE ABLE TO CHANGE FLOORS AND LAND ON NEW COLLISION WITHOUT CHANGE FLOORS AGAIN
		// GOTTA ADD NEW COLLIDER TO THE LIST OF COLLIDER STAYERS AND IGNORE ENTER TRIGGER

		//move(TILESIZE + (TILESIZE * velocity.x), TILESIZE + (TILESIZE * velocity.y));
		moveTo(source.getAABB().left + TILESIZE * 1.04f, source.getAABB().top + TILESIZE * 0.825f);
		//move(TILESIZE * 0.75f * velocity.x, TILESIZE * 0.5f * velocity.y);
		//move(TILESIZE + (source.getAABB().width * velocity.x), TILESIZE*0.81f + ((source.getAABB().height/2) * velocity.y));
	}
	else if (source.getName().compare("stair_up") == 0 || source.getName().compare("ladder_up") == 0) {
		floor = floor + 1;
		changingFloors = true;
		// move entity(player) according to hole/stairs position
		// must be done in server also, and has to be done in a way that player dont
		// walk back down unintentionally

		//move(TILESIZE * velocity.x, TILESIZE * velocity.y);
		moveTo(source.getAABB().left - TILESIZE * 0.9f, source.getAABB().top - TILESIZE * 1.1f);
		//move(TILESIZE*0.75f * velocity.x, TILESIZE * 0.75f * velocity.y);
	}
}

void Entity::onCollisionStay(tmx::Object source) {
	//std::cout << "OnCollisionStay() called" << std::endl;
}

void Entity::onCollisionExit(tmx::Object source) {
	//std::cout << "OnCollisionExit() called" << std::endl;
}


/// gets player current floor as an array index to be used in layer arrays
int Entity::getFloorIdx()
{
	// make sure entity is within min and max floors
	if (floor < MIN_FLOOR) floor = MIN_FLOOR;
	if (floor > MAX_FLOOR) floor = MAX_FLOOR;
	// convert and return array idx
	return floor  + abs(MIN_FLOOR);
}

//  gets collider of entity
sf::Rect<float> Entity::getCollider() {	return collider;}

// sets default collider offsets
void Entity::adjustCollider() {
	colOffset.left = 0; colOffset.top = sprite.getGlobalBounds().width * 0.25f;
	colOffset.width = sprite.getGlobalBounds().width * 0.5f;
	colOffset.height = sprite.getGlobalBounds().height * 0.25f;
	updateCollider();
}

// sets collider offsets and dimensions
void Entity::adjustCollider(float offX, float offY, float scaleX, float scaleY) {
	colOffset.left = offX; colOffset.top = offY;
	colOffset.width = sprite.getGlobalBounds().width * scaleX;
	colOffset.height = sprite.getGlobalBounds().height * scaleY;
	updateCollider();
}

int Entity::load(std::string texturePath, float scaleX, float scaleY)
{
	if (!texture.loadFromFile(texturePath)) {
		std::cout << "Could not load player texture" << std::endl;
		return -1;
	}

	texture.setSmooth(true);
	sprite.setTexture(texture);
	sprite.scale(sf::Vector2f(scaleX, scaleY));

	adjustCollider();

	return 0;
}

// updates collider with entities position
void Entity::updateCollider() {
	collider.left = pos.x + colOffset.left;
	collider.top = pos.y + colOffset.top;
	collider.width = colOffset.width;
	collider.height = colOffset.height;

	// updates list of map collisions removing from it when collision is not happening anymore
	for (int i = collisionObjects.size()-1; i >= 0; i--) {
		const auto& o = collisionObjects.at(i);
		bool col = util::checkRectCollision(collider.left, collider.top, collider.width, collider.height,
			o.getAABB().left, o.getAABB().top, o.getAABB().width, o.getAABB().height);
		if (!col) { // if collision does not happen
			collisionObjects.erase(collisionObjects.begin() + i); // remove from list os collisions
			onCollisionExit(o); // calls on collision exit callback
		}
		else { // if collision still happens, call on collision stay
			onCollisionStay(o);
		}
	}
}

// updates entity 
void Entity::update(float dt) {
	// updates fps
	fps = 1.0f / dt;

	// calculates distance
	float distance = util::distance(pos.x, pos.y, goal.x, goal.y);

	// moves entity direct to goal destination if it is too far
	if (distance > 20.f)
		this->moveTo(goal.x, goal.y);
	else // moves incrementally for interpolation between server snapshots
		this->move(incX, incY);

	// updates entity collider
	updateCollider();
}

// draws entity
void Entity::draw(sf::RenderWindow& window) {
	// centers entity based on its texture size
	sprite.setPosition(pos.x - getCenterOffset().x, getPos().y - getCenterOffset().y);
	// draw entity
	window.draw(sprite);
	
	// if set on (for debug), draw collider of entity
	if (DRAW_COLLIDERS) {
		util::drawRect(window, collider, sf::Color::Blue);
	}
}
