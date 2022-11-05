#include "Entity.h"
#include "Util.h"
#include <iostream>

float incX, incY; // interpolation increments
float fps; // current game fps

Entity::Entity() {
	health = 100; maxHealth = 100; speed = 100; pos = sf::Vector2f(0, 0); velocity = sf::Vector2f(0, 0);
	active = false;
}

Entity::Entity(int health, int maxHealth, int speed, sf::Vector2f initialPos):
	health(health), maxHealth(maxHealth), speed(speed), pos(initialPos)
{
	velocity = sf::Vector2f(0, 0);
	active = false;
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

void Entity::load(sf::Texture& texture) {
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
}

// draws entity
void Entity::draw(sf::RenderWindow& window) {
	// centers entity based on its texture size
	sprite.setPosition(pos.x - getCenterOffset().x, getPos().y - getCenterOffset().y);
	// draw entity
	window.draw(sprite);
}
