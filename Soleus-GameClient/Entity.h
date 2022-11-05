#include <SFML\System\Vector2.hpp>
#include <SFML\Graphics\Sprite.hpp>
#include <SFML\Graphics\Texture.hpp>
#include <SFML\Graphics\RenderWindow.hpp>

#ifndef ENTITY_H
#define ENTITY_H

class Entity
{
	public:
		int health;		// current health of entity
		int maxHealth;  // max health of entity
		int speed;		// entity speed
		sf::Vector2f pos, goal; // position of entity in the world
		sf::Vector2f velocity; // current velocity of entity
		sf::Sprite sprite; // entity current sprite
		sf::Texture texture; // entity texture
		bool active; // is entity active 

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
		sf::Vector2f getVelocity(); // gets entity velocity
		sf::Vector2f getCenterOffset(); // gets entity center offset in relation to top-left position point of sprite
		sf::Sprite getSprite(); // gets entity current sprite
		void setVelocity(sf::Vector2f velocity); // sets entity velocity
		void setVelocity(float vX, float vY); // sets entity velocity 
		void move(sf::Vector2f offset); // moves entity by desired offset (Vector2f)
		void move(float offsetX, float offsetY); // moves entity by desired offset (float values)
		void moveTo(float x, float y); // moves entity to desired position
		void interpolate(float x, float y); // prepares interpolation between current position and position received from server
		void interpolate(sf::Vector2f goal); // prepares interpolation between current position and position received from server
		int react(Entity source, Event event); // reacts to events directed to the entity
		void load(sf::Texture& texture); // loads entity
		void update(float dt); // updates entity in each cycle of the game loop
		void draw(sf::RenderWindow& window); // draws entity in window received in param
};

#endif

