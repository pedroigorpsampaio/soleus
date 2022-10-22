#include "Entity.h"

Entity::Entity() {
}

Entity::Entity(int health, int maxHealth, int speed, sf::Vector2f initialPos):
	health(health), maxHealth(maxHealth), speed(speed), pos(initialPos)
{}

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

sf::Sprite Entity::getSprite()
{
	return sprite;
}

void Entity::move(sf::Vector2f offset)
{
}

void Entity::move(float offsetX, float offsetY)
{
}

int Entity::react(Entity source, Event event)
{
	return 0;
}