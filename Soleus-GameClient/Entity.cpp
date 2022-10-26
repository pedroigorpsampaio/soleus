#include "Entity.h"
#include <cmath>

template<typename _Float>
constexpr std::enable_if_t<std::is_floating_point_v<_Float>, _Float>
lerp(_Float __a, _Float __b, _Float __t)
{
	if (std::isnan(__a) || std::isnan(__b) || std::isnan(__t))
		return std::numeric_limits<_Float>::quiet_NaN();
	else if ((__a <= _Float{ 0 } && __b >= _Float{ 0 })
		|| (__a >= _Float{ 0 } && __b <= _Float{ 0 }))
		// ab <= 0 but product could overflow.
#ifndef FMA
		return __t * __b + (_Float{ 1 } -__t) * __a;
#else
		return std::fma(__t, __b, (_Float{ 1 } -__t) * __a);
#endif
	else if (__t == _Float{ 1 })
		return __b;
	else
	{ // monotonic near t == 1.
#ifndef FMA
		const auto __x = __a + __t * (__b - __a);
#else
		const auto __x = std::fma(__t, __b - __a, __a);
#endif
		return (__t > _Float{ 1 }) == (__b > __a)
			? std::max(__b, __x)
			: std::min(__b, __x);
	}
}

Entity::Entity() {
	health = 100; maxHealth = 100; speed = 100; pos = sf::Vector2f(0,0);
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

int Entity::react(Entity source, Event event)
{
	return 0;
}