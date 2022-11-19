#ifndef COLLIDABLE_H
#define COLLIDABLE_H

#include <tmxlite\Object.hpp>

// An abstract class that should be used for collidable entities
// in addiction to the collider class
class Collidable {
	public:
		virtual void onCollisionEnter(tmx::Object source) = 0; // reacts to collisions with object tiles from map (when entering)
		virtual void onCollisionStay(tmx::Object source) = 0; //  reacts to collisions with object tiles from map (when staying)
		virtual void onCollisionExit(tmx::Object source) = 0; //  reacts to collisions with object tiles from map (when exiting)
};

#endif
