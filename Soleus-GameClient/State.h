#include "Entity.h"
#ifndef STATE_H
#define STATE_H

class State {
	public:
		int nTiles, nEntities;			/// numbers of any kind of object that will need to be packed and unpacked
										/// to aid in the process of unpacking and packing

		std::vector<int> tiles;			/// list of tiles that are in players field of vision / area of interest
		std::vector<Entity> entities;	/// list of entities that are in players field of vision / area if interest
		Entity player;					/// the current state of the player in the server
};

#endif
