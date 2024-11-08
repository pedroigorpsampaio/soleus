#include "Entity.h"
#include <iostream>

float incX, incY; // interpolation increments
float fps; // current game fps

Entity::Entity() {
	health = 100; maxHealth = 100; speed = 100; pos = sf::Vector2f(0, 0); velocity = sf::Vector2f(0, 0);
	active = false; floor = INIT_FLOOR;
	collider.registerOwner(this);
}

Entity::Entity(int health, int maxHealth, int speed, sf::Vector2f initialPos):
	health(health), maxHealth(maxHealth), speed(speed), pos(initialPos)
{
	velocity = sf::Vector2f(0, 0);
	active = false; floor = INIT_FLOOR;
	collider.registerOwner(this);
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

void Entity::onCollisionEnter(tmx::Object source) {
	// default on collision enter for entities
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

//// sets default collider offsets
//void Entity::adjustCollider() {
//	colOffset.left = 0; colOffset.top = sprite.getGlobalBounds().width * 0.25f;
//	colOffset.width = sprite.getGlobalBounds().width * 0.5f;
//	colOffset.height = sprite.getGlobalBounds().height * 0.25f;
//	collider.updateCollider(getPos());
//}
//
//// sets collider offsets and dimensions
//void Entity::adjustCollider(float offX, float offY, float scaleX, float scaleY) {
//	colOffset.left = offX; colOffset.top = offY;
//	colOffset.width = sprite.getGlobalBounds().width * scaleX;
//	colOffset.height = sprite.getGlobalBounds().height * scaleY;
//	collider.updateCollider(getPos());
//}

int Entity::load(std::string texturePath, float scaleX, float scaleY)
{
	if (!texture.loadFromFile(texturePath)) {
		std::cout << "Could not load player texture" << std::endl;
		return -1;
	}

	texture.setSmooth(true);
	sprite.setTexture(texture);
	sprite.setScale(sf::Vector2f(scaleX, scaleY));

	collider.adjustCollider(0, sprite.getGlobalBounds().width * 0.25f,
		sprite.getGlobalBounds().width * 0.5f, sprite.getGlobalBounds().height * 0.25f);

	collider.updateCollider(getPos());

	return 0;
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
	collider.updateCollider(getPos());
}

// draws entity in a texture, relative to camera
void Entity::draw(sf::RenderTarget& rt, sf::Vector2f camera) {
	// centers entity based on its texture size
	sprite.setPosition(pos.x - getCenterOffset().x - camera.x, 
						pos.y - getCenterOffset().y - camera.y);
	
	// draw entity
	rt.draw(sprite);

	// if set on (for debug), draw collider of entity
	if (DRAW_COLLIDERS) {
		sf::FloatRect colScreen(collider.getRect().left - camera.x,
								collider.getRect().top - camera.y,
								collider.getRect().width, collider.getRect().height);
		util::drawRect(rt, colScreen, sf::Color::Blue);
	}
}

// draws entity
void Entity::draw(sf::RenderWindow& window) {
	// centers entity based on its texture size
	sprite.setPosition(pos.x - getCenterOffset().x, pos.y - getCenterOffset().y);
	// draw entity
	window.draw(sprite);
	
	// if set on (for debug), draw collider of entity
	if (DRAW_COLLIDERS) {
		util::drawRect(window, collider.getRect(), sf::Color::Blue);
	}
}
