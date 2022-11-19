#ifndef COLLIDER_H
#define COLLIDER_H

#include <SFML\Graphics\Rect.hpp>
#include <vector>
#include <tmxlite\Object.hpp>
#include "Collidable.h"

/// Represents a physical form that can collide with other physical
/// forms, in a physical manner (blocking movement) or in a trigger
/// manner (triggering events when colliding with other forms)
class Collider {
	public:
		/// constructor
		Collider();
		void registerOwner(Collidable *owner);
		/// setups collider with coordinates, dimensions and flags
		void setup(float left, float top, float width, float height, bool trigger);

		sf::Rect<float> getRect(); /// gets collider rectangle
		void setRect(float left, float top, float width, float height); /// sets collider rectangle
		bool isTrigger(); /// gets flag that says if it is a trigger collider or a phys one
		void setTrigger(bool isTrigger); /// sets flag that says if it is a trigger collider or a phys one

		void checkCollision(tmx::Object object); /// checks collision with an map object
		void checkCollision(tmx::Object object, sf::Vector2f move); /// checks collision with an map object (with entity collision move offset)
		bool checkPhysCollision(tmx::Object object); /// checks physical collision with an map object (blocks movement)
		bool checkPhysCollision(tmx::Object object, sf::Vector2f move); /// checks collision with an map object (with entity collision move offset)  (blocks movement)
		void updateCollider(sf::Vector2f pos); /// updates collider position based on owner current position
		void adjustCollider(); /// adjust collider with dafault offsets and scales
		void adjustCollider(float offX, float offY, float scaleX, float scaleY); /// adjust collider with received offsets and scales

	private:
		Collidable *owner; /// collidable owner of this collider
		sf::Rect<float> rect; /// collider physical rectangle
		std::vector<tmx::Object> collisionObjects; /// list of current map object collisions with this collider
		bool trigger; /// is this a trigger collider?
		sf::Rect<float> colOffset; /// offsets for collider adjustment (x and y offset, width and height of collider)
};

#endif

