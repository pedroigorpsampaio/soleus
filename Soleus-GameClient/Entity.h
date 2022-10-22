#include <SFML\System\Vector2.hpp>
#include <SFML\Graphics\Sprite.hpp>
#include <SFML\Graphics\Texture.hpp>

#ifndef ENTITY_H
#define ENTITY_H

class Entity
{

	protected:
		int health;		// current health of entity
		int maxHealth;  // max health of entity
		int speed;		// entity speed
		sf::Vector2f pos; // position of entity in the world
		sf::Sprite sprite; // entity current sprite
		sf::Texture texture; // entity texture

	public:
		enum Event {	// enum that represent possible events that can happen to entities
			MoveUp = 1 << 0,
			MoveDown = 1 << 1,
			MoveRight = 1 << 2,
			MoveLeft = 1 << 3,
		};
		Entity(); // empty constructor
		Entity(int health, int maxHealth, int speed, sf::Vector2f initialPos); // constructor with attributes
		int getHealth(); // gets entity health
		int getMaxHealth(); // gets entity max health
		int getSpeed(); // gets entity speed
		sf::Vector2f getPos(); // gets entity position in the world
		sf::Sprite getSprite(); // gets entity current sprite
		void move(sf::Vector2f offset); // moves entity by desired offset (Vector2f)
		void move(float offsetX, float offsetY); // moves entity by desired offset (float values)
		int react(Entity source, Event event); // reacts to events directed to the entity
};

#endif

