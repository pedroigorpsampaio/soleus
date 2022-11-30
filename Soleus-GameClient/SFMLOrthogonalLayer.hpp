/*********************************************************************
(c) Matt Marchant & contributors 2016 - 2019
http://trederia.blogspot.com

tmxlite - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

/*
Creates an SFML drawable from an Orthogonal tmx map layer.
This is an example of drawing with SFML - not all features
are implemented.
*/

#ifndef SFML_ORTHO_HPP_
#define SFML_ORTHO_HPP_

#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/detail/Log.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Time.hpp>

#include <memory>
#include <vector>
#include <array>
#include <map>
#include <string>
#include <limits>
#include <iostream>
#include <cmath>

#include "Util.h"

namespace LayerType
{
	/// Values that represent the different types of layer
	enum {
		//---------------------------------------------
		// Type                      
		//---------------------------------------------
		Floor,   /// layer that contains floor tiles - grass, stone, etc... -> basic ground tiles
		Wall,	/// layer that contains floor walls - walls of houses, bridges, any wall-like tile -> blocks user movement
		Inbetween, /// layer that contain floor misc tiles that are inbetween two layers - trees, decorations, stairs, etc...
		Collision,  /// object layer containing collision objects of floor - for physical collision check
		Stair, /// object layer containing stair trigger objects of floor - for changing floors trigger actions
	};
}

class MapLayer final : public sf::Drawable
{
public:
	std::vector<tmx::Object> objects;
	std::vector<tmx::Object> visibleObjects;
	std::vector<tmx::TileLayer::Chunk> chunks;
	std::vector<const tmx::Tileset*> usedTileSets;
	const sf::VertexArray *m_vertices;
	const sf::Texture *m_tileset;

	const tmx::Map *map;
	std::size_t idx;

	mutable Player *player; // reference to player
	mutable sf::Vector2f camera; // game camera

	MapLayer(const tmx::Map& map, std::size_t idx)
	{
		const auto& layers = map.getLayers();
		this->map = &map;
		this->idx = idx;

		// gets layerproperties to be stored
		const auto& layerProperties = layers[idx]->getProperties();

		// loops through properties to store them accordingly
		for (const auto& prop : layerProperties)
		{
			std::string propName = prop.getName();
			if (propName.compare("floor") == 0 && prop.getType() == tmx::Property::Type::Int) {
				m_floor = prop.getIntValue();
			}
			else if (propName.compare("type") == 0 && prop.getType() == tmx::Property::Type::String) {
				std::string type = prop.getStringValue();
				if (type.compare("floor") == 0) {
					m_layerType = LayerType::Floor;
				}
				else if (type.compare("wall") == 0) {
					m_layerType = LayerType::Wall;
				}
				else if (type.compare("inbetween") == 0) {
					m_layerType = LayerType::Inbetween;
				}
				else {
					std::cout << "tile layer with unknown type loaded" << std::endl;
				}
			}
		}

		// tile layers
		if (map.getOrientation() == tmx::Orientation::Orthogonal &&
			idx < layers.size() && layers[idx]->getType() == tmx::Layer::Type::Tile)
		{
			m_isObjectLayer = false;
			////round the chunk size to the nearest tile
			const auto tileSize = map.getTileSize();
			m_chunkSize.x = std::floor(m_chunkSize.x / tileSize.x) * tileSize.x;
			m_chunkSize.y = std::floor(m_chunkSize.y / tileSize.y) * tileSize.y;
			m_MapTileSize.x = map.getTileSize().x;
			m_MapTileSize.y = map.getTileSize().y;
			const auto& layer = layers[idx]->getLayerAs<tmx::TileLayer>();
			this->chunks = layers[idx]->getLayerAs<tmx::TileLayer>().getChunks();
			createChunks(map, layer);
			auto mapSize = map.getBounds();
			m_globalBounds.width = mapSize.width;
			m_globalBounds.height = mapSize.height;
		}
		else // object layers
		{
			m_isObjectLayer = true;
			//std::cout << "Not a valid orthogonal layer, nothing will be drawn." << std::endl;

			const tmx::ObjectGroup& objectLayer = layers[idx]->getLayerAs<tmx::ObjectGroup>();
			objects = objectLayer.getObjects();
			const auto& objectLayerProperties = objectLayer.getProperties();

			// loops through properties to store them accordingly
			for (const auto& prop : objectLayerProperties)
			{
				std::string propName = prop.getName();
				if (propName.compare("floor") == 0 && prop.getType() == tmx::Property::Type::Int) {
					m_floor = prop.getIntValue();
				}
				else if (propName.compare("type") == 0 && prop.getType() == tmx::Property::Type::String) {
					std::string type = prop.getStringValue();
					if (type.compare("collision") == 0) {
						m_layerType = LayerType::Collision;
					}
					else if (type.compare("stair") == 0) {
						m_layerType = LayerType::Stair;
					}
					else {
						std::cout << "object layer with unknown type loaded" << std::endl;
					}
				}
			}

			//for (const tmx::Object object : objects)
			//{
			//	// do stuff with object properties
			//}
		}
	}

	~MapLayer() = default;
	MapLayer(const MapLayer&) = delete;
	MapLayer& operator = (const MapLayer&) = delete;

	const sf::FloatRect& getGlobalBounds() const { return m_globalBounds; }

	int getFloor() { return m_floor; } 	/// returns the floor this layer is tied to
	int setFloor(int floor) { m_floor = floor; } /// ties this layer to its floor
	bool isObjectLayer() { return m_isObjectLayer; } /// returns if this layer is of object type
	int getLayerType() { return m_layerType; } /// returns this layer type 

	void setTile(int tileX, int tileY, tmx::TileLayer::Tile tile, bool refresh = true)
	{
		sf::Vector2u chunkLocale;
		const auto& selectedChunk = getChunkAndTransform(tileX, tileY, chunkLocale);
		selectedChunk->setTile(chunkLocale.x, chunkLocale.y, tile, refresh);
	}

	tmx::TileLayer::Tile getTile(int tileX, int tileY)
	{
		sf::Vector2u chunkLocale;
		const auto& selectedChunk = getChunkAndTransform(tileX, tileY, chunkLocale);
		return selectedChunk->getTile(chunkLocale.x, chunkLocale.y);
	}
	void setColor(int tileX, int tileY, sf::Color color, bool refresh = true)
	{
		sf::Vector2u chunkLocale;
		const auto& selectedChunk = getChunkAndTransform(tileX, tileY, chunkLocale);
		selectedChunk->setColor(chunkLocale.x, chunkLocale.y, color, refresh);
	}

	sf::Color getColor(int tileX, int tileY)
	{
		sf::Vector2u chunkLocale;
		const auto& selectedChunk = getChunkAndTransform(tileX, tileY, chunkLocale);
		return selectedChunk->getColor(chunkLocale.x, chunkLocale.y);
	}

	void update(sf::Time elapsed)
	{
		for (auto& c : m_visibleChunks)
		{
			for (AnimationState& as : c->getActiveAnimations())
			{
				as.currentTime += elapsed;

				tmx::TileLayer::Tile tile;
				std::uint32_t animTime = 0;
				auto x = as.animTile.animation.frames.begin();
				while (animTime < as.currentTime.asMilliseconds())
				{
					if (x == as.animTile.animation.frames.end())
					{
						x = as.animTile.animation.frames.begin();
						as.currentTime -= sf::milliseconds(animTime);
						animTime = 0;
					}

					tile.ID = x->tileID;
					animTime += x->duration;
					x++;
				}

				setTile(as.tileCords.x, as.tileCords.y, tile);
			}
		}
	}


	//increasing m_chunkSize by 4; fixes render problems when mapsize != chunksize
	//sf::Vector2f m_chunkSize = sf::Vector2f(1024.f, 1024.f);
	sf::Vector2f m_chunkSize = sf::Vector2f(512.f, 512.f);
	sf::Vector2u m_chunkCount;
	sf::Vector2u m_MapTileSize;   // general Tilesize of Map
	sf::FloatRect m_globalBounds;
	int m_floor = 0; // the map floor that this layer is tied to (each floor of the map can have multiple layers)
	bool m_isObjectLayer; // is this layer an object layer?
	int m_layerType; // the type that of tiles/objects this layer contains

	using TextureResource = std::map<std::string, std::unique_ptr<sf::Texture>>;
	TextureResource m_textureResource;

	struct AnimationState
	{
		sf::Vector2u tileCords;
		sf::Time startTime;
		sf::Time currentTime;
		tmx::Tileset::Tile animTile;
		std::uint8_t flipFlags;
	};

	class Chunkie final : public sf::Transformable, public sf::Drawable
	{
	public:
		sf::VertexArray m_vertices;
		sf::Texture *m_tileset;
		sf::Rect<float> rect;
		sf::Vector2f camera; // camera based on players position

		bool load(TextureResource& tilesetResource, std::vector<const tmx::Tileset*> tilesets, 
			sf::Vector2u tileSize, std::vector<tmx::TileLayer::Tile> tiles, 
				unsigned int width, unsigned int height, tmx::Vector2i position)
		{
			rect = sf::Rect<float>(position.x*tileSize.x, position.y*tileSize.y,
									width * tileSize.x, height * tileSize.y);
			//float atlasW, atlasH;
			//for (const auto& ts : tilesets) {
			//	atlasW += ts->getImageSize().x; atlasH += ts->getImageSize().y;
			//}
			//if (!tileAtlas.create(atlasW, atlasH))
			//	std::cout << "error creating tile atlas texture" << std::endl;

			//texture.setSmooth(true);
			//sf::Sprite sprite;

			//for (const auto& ts : tilesets)
			//{
			//	if (ts->getImagePath().empty())
			//	{
			//		tmx::Logger::log("This example does not support Collection of Images tilesets", tmx::Logger::Type::Info);
			//		tmx::Logger::log("Chunks using " + ts->getName() + " will not be created", tmx::Logger::Type::Info);
			//		continue;
			//	}
			//	//*tilesetResource.find(ts->getImagePath())->second;
			//	//tileAtlas.draw(*tilesetResource.find(ts->getImagePath())->second);
			//}

			// resize the vertex array to fit the level size
			m_vertices.setPrimitiveType(sf::Quads);
			m_vertices.resize(width * height * 4);

			// populate the vertex array, with one quad per tile
			for (unsigned int i = 0; i < width; ++i)
				for (unsigned int j = 0; j < height; ++j)
				{
					// get the current tile number
					int tileNumber = tiles[i + j * width].ID;
					// ignore empty tiles
					if (tileNumber == 0) continue;

					// WHY INBETWEENS AND WALLS ARE BUGGY?!
					int tsID;
					for (tsID = tilesets.size() - 1; tsID >= 0; tsID--) {
						if (tileNumber > tilesets.at(tsID)->getFirstGID()) {
							break;
						}
					}

					// fix limits
					if (tsID >= tilesets.size())
						tsID = tilesets.size() - 1;
					if (tsID < 0) tsID = 0;

					m_tileset = &(*tilesetResource.find(tilesets.at(tsID)->getImagePath())->second);
					tileNumber -= tilesets.at(tsID)->getFirstGID();

					//std::cout << tilesets.at(tsID)->getFirstGID() << std::endl;

					// size of tiles in tileset (can be different of map tilesize)
					sf::Vector2u tsTileSize(tilesets.at(tsID)->getTileSize().x, tilesets.at(tsID)->getTileSize().y);
					// tilesize offset in case of differences between tileset and map tilesize
					int offX = tileSize.x - tsTileSize.x; int offY = tileSize.y - tsTileSize.y;
					sf::Vector2u tsOff(abs(offX), abs(offY));

					// find its position in the tileset texture
					int tu = tileNumber % (m_tileset->getSize().x / tsTileSize.x);
					int tv = tileNumber / (m_tileset->getSize().x / tsTileSize.x);

					//if(tilesets.at(tsID)->getFirstGID() > 1)
						//std::cout << "tu: " << tu << " | tv: " << tv << std::endl;

					// get a pointer to the current tile's quad
					sf::Vertex* quad = &m_vertices[(i + j * width) * 4];

					// position of chunk in map (relative to map tilesize)
					sf::Vector2f cPos = sf::Vector2f(position.x * tileSize.x, position.y * tileSize.y);
					//sf::Vector2f oPos = sf::Vector2f(, position.y * tileSize.y);

					// define its 4 texture coordinates
					quad[0].texCoords = sf::Vector2f(tu * tsTileSize.x, tv * tsTileSize.y);
					quad[1].texCoords = sf::Vector2f((tu + 1) * tsTileSize.x, tv * tsTileSize.y);
					quad[2].texCoords = sf::Vector2f((tu + 1) * tsTileSize.x, (tv + 1) * tsTileSize.y);
					quad[3].texCoords = sf::Vector2f(tu * tsTileSize.x, (tv + 1) * tsTileSize.y);

					// define its 4 corners for the position in the world map
					quad[0].position = sf::Vector2f(i * tileSize.x + cPos.x, j * tileSize.y + cPos.y - tsOff.y);
					quad[1].position = sf::Vector2f((i + 1) * tileSize.x + cPos.x + tsOff.x, j * tileSize.y + cPos.y - tsOff.y);
					quad[2].position = sf::Vector2f((i + 1) * tileSize.x + cPos.x + tsOff.x, (j + 1) * tileSize.y + cPos.y);
					quad[3].position = sf::Vector2f(i * tileSize.x + cPos.x, (j + 1) * tileSize.y + cPos.y);

					//quad[0].position = sf::Vector2f(2080, 3008);
					//quad[1].position = sf::Vector2f(2080 + 32, 3008);
					//quad[2].position = sf::Vector2f(2080 + 32, 3008 +32);
					//quad[3].position = sf::Vector2f(2080, 3008 + 32);

					//quad[0].texCoords = sf::Vector2f(0, 0);
					//quad[1].texCoords = sf::Vector2f(0 + 32, 0);
					//quad[2].texCoords = sf::Vector2f(0 + 32, 0 + 32);
					//quad[3].texCoords = sf::Vector2f(0, 0 + 32);

					//std::cout << quad[0].texCoords.x << " | " << quad[0].texCoords.y << std::endl;
						
				}

			return true;
		}

		// updates before drawing map and entities
		void update(sf::View& view, sf::Vector2f camera, Player& player) {
			this->camera.x = camera.x;
			this->camera.y = camera.y;

			// TODO
			// UPDATE VISIBLE CHUNKS TO CHECK COLLISION BETWEEN CHUNKS AND NOW CAMERA RECT (INSTEAD OF VIEW RECT)
			// CORRECT DRAWING OF PLAYER, ENTITIES AND COLLIDERS (NO BLUR ON SCALE!!!)
			// DRAW ORDER - INTERCALATE WALLS/STAIRS/OBJECTS WITH ENTITIES OF THE SAME FLOOR IN VISIBLE CHUNKS !! AAA
			// REPLICATE COLLIDING AND STAIR PHYSICS IN SERVER 
			// SEND VISIBLE CHUNKS DATA TO CLIENT (IS IT A GOOD IDEA?)

			//std::cout << vpAnchor.x << std::endl;
		}

		void draw(sf::RenderTarget& rt, sf::RenderStates states) const override
		{
			// adjust based on game camera that follows player
			sf::Transform t = getTransform();
			t.translate(-camera.x, -camera.y);

			// apply the transform
			states.transform *= t;

			// apply the tileset texture
			states.texture = m_tileset;

			// draw the vertex array
			rt.draw(m_vertices, states);

			//t.translate(camera.x, camera.y);
			//states.transform *= t;
		}
	};

	class Chunk final : public sf::Transformable, public sf::Drawable
	{
	public:
		using Ptr = std::unique_ptr<Chunk>;

		// the Android OpenGL driver isn't capable of rendering quads,
		// so we need to use two triangles per tile instead
#ifdef __ANDROID__
		using Tile = std::array<sf::Vertex, 6u>;
#endif
#ifndef __ANDROID__
		using Tile = std::array<sf::Vertex, 4u>;
#endif
		Chunk(const tmx::TileLayer& layer, std::vector<const tmx::Tileset*> tilesets,
			const sf::Vector2f& position, const sf::Vector2f& tileCount, const sf::Vector2u& tileSize,
			std::size_t rowSize, TextureResource& tr, const std::map<std::uint32_t, tmx::Tileset::Tile>& animTiles)
			: m_animTiles(animTiles)
		{
			setPosition(position);
			layerOpacity = static_cast<sf::Uint8>(layer.getOpacity() / 1.f * 255.f);
			sf::Color vertColour = sf::Color(200, 200, 200, layerOpacity);
			auto offset = layer.getOffset();
			layerOffset.x = offset.x;
			layerOffset.x = offset.y;
			chunkTileCount.x = tileCount.x;
			chunkTileCount.y = tileCount.y;
			mapTileSize = tileSize;
			const auto& tileIDs = layer.getTiles();

			//go through the tiles and create all arrays (for latter manipulation)
			for (const auto& ts : tilesets)
			{
				if (ts->getImagePath().empty())
				{
					tmx::Logger::log("This example does not support Collection of Images tilesets", tmx::Logger::Type::Info);
					tmx::Logger::log("Chunks using " + ts->getName() + " will not be created", tmx::Logger::Type::Info);
					continue;
				}
				m_chunkArrays.emplace_back(std::make_unique<ChunkArray>(*tr.find(ts->getImagePath())->second, *ts));
			}
			std::size_t xPos = static_cast<std::size_t>(position.x / tileSize.x);
			std::size_t yPos = static_cast<std::size_t>(position.y / tileSize.y);
			for (auto y = yPos; y < yPos + tileCount.y; ++y)
			{
				for (auto x = xPos; x < xPos + tileCount.x; ++x)
				{
					auto idx = (y * rowSize + x);
					m_chunkTileIDs.emplace_back(tileIDs[idx]);
					m_chunkColors.emplace_back(vertColour);
				}
			}
			generateTiles(true);
		}

		void generateTiles(bool registerAnimation = false)
		{
			if (registerAnimation)
			{
				m_activeAnimations.clear();
			}
			for (const auto& ca : m_chunkArrays)
			{
				sf::Uint32 idx = 0;
				std::size_t xPos = static_cast<std::size_t>(getPosition().x / mapTileSize.x);
				std::size_t yPos = static_cast<std::size_t>(getPosition().y / mapTileSize.y);
				for (auto y = yPos; y < yPos + chunkTileCount.y; ++y)
				{
					for (auto x = xPos; x < xPos + chunkTileCount.x; ++x)
					{
						if (idx < m_chunkTileIDs.size() && m_chunkTileIDs[idx].ID >= ca->m_firstGID
							&& m_chunkTileIDs[idx].ID <= ca->m_lastGID)
						{
							if (registerAnimation && m_animTiles.find(m_chunkTileIDs[idx].ID) != m_animTiles.end())
							{
								AnimationState as;
								as.animTile = m_animTiles[m_chunkTileIDs[idx].ID];
								as.startTime = sf::milliseconds(0);
								as.tileCords = sf::Vector2u(x, y);
								m_activeAnimations.push_back(as);
							}

							sf::Vector2f tileOffset(x * mapTileSize.x, (float)y * mapTileSize.y + mapTileSize.y - ca->tileSetSize.y);

							auto idIndex = m_chunkTileIDs[idx].ID - ca->m_firstGID;
							sf::Vector2f tileIndex(idIndex % ca->tsTileCount.x, idIndex / ca->tsTileCount.x);
							tileIndex.x *= ca->tileSetSize.x;
							tileIndex.y *= ca->tileSetSize.y;
							Tile tile =
							{
#ifndef __ANDROID__
								sf::Vertex(tileOffset - getPosition(), m_chunkColors[idx], tileIndex),
								sf::Vertex(tileOffset - getPosition() + sf::Vector2f(ca->tileSetSize.x, 0.f), m_chunkColors[idx], tileIndex + sf::Vector2f(ca->tileSetSize.x, 0.f)),
								sf::Vertex(tileOffset - getPosition() + sf::Vector2f(ca->tileSetSize.x, ca->tileSetSize.y), m_chunkColors[idx], tileIndex + sf::Vector2f(ca->tileSetSize.x, ca->tileSetSize.y)),
								sf::Vertex(tileOffset - getPosition() + sf::Vector2f(0.f, ca->tileSetSize.y), m_chunkColors[idx], tileIndex + sf::Vector2f(0.f, ca->tileSetSize.y))
#endif
#ifdef __ANDROID__
								sf::Vertex(tileOffset - getPosition(), m_chunkColors[idx], tileIndex),
								sf::Vertex(tileOffset - getPosition() + sf::Vector2f(ca->tileSetSize.x, 0.f), m_chunkColors[idx], tileIndex + sf::Vector2f(ca->tileSetSize.x, 0.f)),
								sf::Vertex(tileOffset - getPosition() + sf::Vector2f(ca->tileSetSize.x, ca->tileSetSize.y), m_chunkColors[idx], tileIndex + sf::Vector2f(ca->tileSetSize.x, ca->tileSetSize.y)),
								sf::Vertex(tileOffset - getPosition(), m_chunkColors[idx], tileIndex),
								sf::Vertex(tileOffset - getPosition() + sf::Vector2f(0.f, ca->tileSetSize.y), m_chunkColors[idx], tileIndex + sf::Vector2f(0.f, ca->tileSetSize.y)),
								sf::Vertex(tileOffset - getPosition() + sf::Vector2f(ca->tileSetSize.x, ca->tileSetSize.y), m_chunkColors[idx], tileIndex + sf::Vector2f(ca->tileSetSize.x, ca->tileSetSize.y))
#endif
							};
							doFlips(m_chunkTileIDs[idx].flipFlags, &tile[0].texCoords, &tile[1].texCoords, &tile[2].texCoords, &tile[3].texCoords);
							ca->addTile(tile);
						}
						idx++;
					}
				}
			}
		}
		~Chunk() = default;
		Chunk(const Chunk&) = delete;
		Chunk& operator = (const Chunk&) = delete;
		std::vector<AnimationState>& getActiveAnimations() { return m_activeAnimations; }
		tmx::TileLayer::Tile getTile(int x, int y) const
		{
			return m_chunkTileIDs[calcIndexFrom(x, y)];
		}
		void setTile(int x, int y, tmx::TileLayer::Tile tile, bool refresh)
		{
			m_chunkTileIDs[calcIndexFrom(x, y)] = tile;
			maybeRegenerate(refresh);
		}
		sf::Color getColor(int x, int y) const
		{
			return m_chunkColors[calcIndexFrom(x, y)];
		}
		void setColor(int x, int y, sf::Color color, bool refresh)
		{
			m_chunkColors[calcIndexFrom(x, y)] = color;
			maybeRegenerate(refresh);
		}
		void maybeRegenerate(bool refresh)
		{
			if (refresh)
			{
				for (const auto& ca : m_chunkArrays)
				{
					ca->reset();
				}
				generateTiles();
			}
		}
		int calcIndexFrom(int x, int y) const
		{
			return x + y * chunkTileCount.x;
		}
		bool empty() const { return m_chunkArrays.empty(); }
		void flipY(sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3)
		{
			//Flip Y
			sf::Vector2f tmp = *v0;
			v0->y = v2->y;
			v1->y = v2->y;
			v2->y = tmp.y;
			v3->y = v2->y;
		}

		void flipX(sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3)
		{
			//Flip X
			sf::Vector2f tmp = *v0;
			v0->x = v1->x;
			v1->x = tmp.x;
			v2->x = v3->x;
			v3->x = v0->x;
		}

		void flipD(sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3)
		{
			//Diagonal flip
			sf::Vector2f tmp = *v1;
			v1->x = v3->x;
			v1->y = v3->y;
			v3->x = tmp.x;
			v3->y = tmp.y;
		}

		void doFlips(std::uint8_t bits, sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3)
		{
			//0000 = no change
			//0100 = vertical = swap y axis
			//1000 = horizontal = swap x axis
			//1100 = horiz + vert = swap both axes = horiz+vert = rotate 180 degrees
			//0010 = diag = rotate 90 degrees right and swap x axis
			//0110 = diag+vert = rotate 270 degrees right
			//1010 = horiz+diag = rotate 90 degrees right
			//1110 = horiz+vert+diag = rotate 90 degrees right and swap y axis
			if (!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
				!(bits & tmx::TileLayer::FlipFlag::Vertical) &&
				!(bits & tmx::TileLayer::FlipFlag::Diagonal))
			{
				//Shortcircuit tests for nothing to do
				return;
			}
			else if (!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
				(bits & tmx::TileLayer::FlipFlag::Vertical) &&
				!(bits & tmx::TileLayer::FlipFlag::Diagonal))
			{
				//0100
				flipY(v0, v1, v2, v3);
			}
			else if ((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
				!(bits & tmx::TileLayer::FlipFlag::Vertical) &&
				!(bits & tmx::TileLayer::FlipFlag::Diagonal))
			{
				//1000
				flipX(v0, v1, v2, v3);
			}
			else if ((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
				(bits & tmx::TileLayer::FlipFlag::Vertical) &&
				!(bits & tmx::TileLayer::FlipFlag::Diagonal))
			{
				//1100
				flipY(v0, v1, v2, v3);
				flipX(v0, v1, v2, v3);
			}
			else if (!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
				!(bits & tmx::TileLayer::FlipFlag::Vertical) &&
				(bits & tmx::TileLayer::FlipFlag::Diagonal))
			{
				//0010
				flipD(v0, v1, v2, v3);
			}
			else if (!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
				(bits & tmx::TileLayer::FlipFlag::Vertical) &&
				(bits & tmx::TileLayer::FlipFlag::Diagonal))
			{
				//0110
				flipX(v0, v1, v2, v3);
				flipD(v0, v1, v2, v3);
			}
			else if ((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
				!(bits & tmx::TileLayer::FlipFlag::Vertical) &&
				(bits & tmx::TileLayer::FlipFlag::Diagonal))
			{
				//1010
				flipY(v0, v1, v2, v3);
				flipD(v0, v1, v2, v3);
			}
			else if ((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
				(bits & tmx::TileLayer::FlipFlag::Vertical) &&
				(bits & tmx::TileLayer::FlipFlag::Diagonal))
			{
				//1110
				flipY(v0, v1, v2, v3);
				flipX(v0, v1, v2, v3);
				flipD(v0, v1, v2, v3);
			}
		}

	
		class ChunkArray final : public sf::Drawable
		{
		public:
			using Ptr = std::unique_ptr<ChunkArray>;
			tmx::Vector2u tileSetSize;
			sf::Vector2u tsTileCount;
			std::uint32_t m_firstGID, m_lastGID;
			explicit ChunkArray(const sf::Texture& t, const tmx::Tileset& ts)
				: m_texture(t)
			{
				auto texSize = getTextureSize();
				tileSetSize = ts.getTileSize();
				tsTileCount.x = texSize.x / tileSetSize.x;
				tsTileCount.y = texSize.y / tileSetSize.y;
				m_firstGID = ts.getFirstGID();
				m_lastGID = ts.getLastGID();
			}

			~ChunkArray() = default;
			ChunkArray(const ChunkArray&) = delete;
			ChunkArray& operator = (const ChunkArray&) = delete;

			void reset()
			{
				m_vertices.clear();
			}
			void addTile(const Chunk::Tile& tile)
			{
				for (const auto& v : tile)
				{
					m_vertices.push_back(v);
				}
			}
			sf::Vector2u getTextureSize() const { return m_texture.getSize(); }

		
			const sf::Texture& m_texture;
			std::vector<sf::Vertex> m_vertices;
			void draw(sf::RenderTarget& rt, sf::RenderStates states) const override
			{
				states.texture = &m_texture;
#ifndef __ANDROID__
				rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
#endif
#ifdef __ANDROID__
				rt.draw(m_vertices.data(), m_vertices.size(), sf::Triangles, states);
#endif
			}
		};

		sf::Uint8 layerOpacity;     // opacity of the layer
		sf::Vector2f layerOffset;   // Layer offset
		sf::Vector2u mapTileSize;   // general Tilesize of Map
		sf::Vector2f chunkTileCount;   // chunk tilecount
		std::vector<tmx::TileLayer::Tile> m_chunkTileIDs; // stores all tiles in this chunk for later manipulation
		std::vector<sf::Color> m_chunkColors; // stores colors for extended color effects
		std::map<std::uint32_t, tmx::Tileset::Tile> m_animTiles;    // animation catalogue
		std::vector<AnimationState> m_activeAnimations;     // Animations to be done in this chunk
		std::vector<ChunkArray::Ptr> m_chunkArrays;
		void draw(sf::RenderTarget& rt, sf::RenderStates states) const override
		{
			states.transform *= getTransform();
			for (const auto& a : m_chunkArrays)
			{
				rt.draw(*a, states);
			}
		}
	};

	std::vector<Chunk::Ptr> m_chunks;
	mutable std::vector<Chunkie> m_chunkies;
	mutable std::vector<Chunkie> m_visibleChunkies;
	mutable std::vector<Chunk*> m_visibleChunks;

	std::vector<Chunkie>& getVisibleChunks() {
		return m_visibleChunkies;
	}

	Chunk::Ptr& getChunkAndTransform(int x, int y, sf::Vector2u& chunkRelative)
	{
		uint32_t chunkX = floor((x * m_MapTileSize.x) / m_chunkSize.x);
		uint32_t chunkY = floor((y * m_MapTileSize.y) / m_chunkSize.y);
		chunkRelative.x = ((x * m_MapTileSize.x) - chunkX * m_chunkSize.x) / m_MapTileSize.x;
		chunkRelative.y = ((y * m_MapTileSize.y) - chunkY * m_chunkSize.y) / m_MapTileSize.y;
		return  m_chunks[chunkX + chunkY * m_chunkCount.x];
	}
	void createChunks(const tmx::Map& map, const tmx::TileLayer& layer)
	{
		//look up all the tile sets and load the textures
		const auto& tileSets = map.getTilesets();

		for (auto i = 0; i < tileSets.size(); i++)
		{
			usedTileSets.push_back(&(tileSets.at(i)));
		}

		//for (const auto& c : chunks) {
		//	const auto& layerIDs = c.tiles;
		//	std::uint32_t maxID = std::numeric_limits<std::uint32_t>::max();

		//	for (auto i = tileSets.rbegin(); i != tileSets.rend(); ++i)
		//	{
		//		for (const auto& tile : layerIDs)
		//		{
		//			if (tile.ID >= i->getFirstGID() && tile.ID < maxID)
		//			{
		//				usedTileSets.push_back(&(*i));
		//				break;
		//			}
		//		}
		//		maxID = i->getFirstGID();
		//	}
		//}


		sf::Image fallback;
		fallback.create(2, 2, sf::Color::Magenta);
		for (const auto& ts : usedTileSets)
		{
			const auto& path = ts->getImagePath();
			//std::unique_ptr<sf::Texture> newTexture = std::make_unique<sf::Texture>();
			std::unique_ptr<sf::Texture> newTexture = std::make_unique<sf::Texture>();
			sf::Image img;
			if (!img.loadFromFile(path))
			{
				newTexture->loadFromImage(fallback);
			}
			else
			{
				if (ts->hasTransparency())
				{
					auto transparency = ts->getTransparencyColour();
					img.createMaskFromColor({ transparency.r, transparency.g, transparency.b, transparency.a });
				}
				newTexture->loadFromImage(img);
			}
			m_textureResource.insert(std::make_pair(path, std::move(newTexture)));
		}

		sf::Vector2u tileSize(map.getTileSize().x, map.getTileSize().y);

		for (const auto& c : chunks) {
			Chunkie chunkie;
			chunkie.load(m_textureResource, usedTileSets, tileSize, c.tiles, c.size.x, c.size.y, c.position);
			m_chunkies.push_back(chunkie);
		}

		//calculate the number of chunks in the layer
		//and create each one
		//const auto bounds = map.getBounds();
		//m_chunkCount.x = static_cast<sf::Uint32>(std::ceil(bounds.width / m_chunkSize.x));
		//m_chunkCount.y = static_cast<sf::Uint32>(std::ceil(bounds.height / m_chunkSize.y));

		//sf::Vector2u tileSize(map.getTileSize().x, map.getTileSize().y);

		//for (auto y = 0u; y < m_chunkCount.y; ++y)
		//{
		//	sf::Vector2f tileCount(m_chunkSize.x / tileSize.x, m_chunkSize.y / tileSize.y);
		//	for (auto x = 0u; x < m_chunkCount.x; ++x)
		//	{
		//		// calculate size of each Chunk (clip against map)
		//		if ((x + 1) * m_chunkSize.x > bounds.width)
		//		{
		//			tileCount.x = (bounds.width - x * m_chunkSize.x) / map.getTileSize().x;
		//		}
		//		if ((y + 1) * m_chunkSize.y > bounds.height)
		//		{
		//			tileCount.y = (bounds.height - y * m_chunkSize.y) / map.getTileSize().y;
		//		}
		//		//m_chunks.emplace_back(std::make_unique<Chunk>(layer, usedTileSets,
		//		//    sf::Vector2f(x * m_chunkSize.x, y * m_chunkSize.y), tileCount, map.getTileCount().x, m_textureResource));
		////		m_chunks.emplace_back(std::make_unique<Chunk>(layer, usedTileSets,
		////			sf::Vector2f(x * m_chunkSize.x, y * m_chunkSize.y), tileCount, tileSize, map.getTileCount().x, m_textureResource, map.getAnimatedTiles()));
		//	}
		//}
	}

	void updateVisibility(const sf::View& view) const
	{
		sf::Vector2f viewCorner = view.getCenter();
		viewCorner -= view.getSize() / 2.f;

		int posX = static_cast<int>(std::floor(viewCorner.x / m_chunkSize.x));
		int posY = static_cast<int>(std::floor(viewCorner.y / m_chunkSize.y));
		int posX2 = static_cast<int>(std::ceil((viewCorner.x + view.getSize().x) / m_chunkSize.x));
		int posY2 = static_cast<int>(std::ceil((viewCorner.y + view.getSize().x) / m_chunkSize.y));

		std::vector<Chunk*> visible;
		for (auto y = posY; y < posY2; ++y)
		{
			for (auto x = posX; x < posX2; ++x)
			{
				std::size_t idx = y * int(m_chunkCount.x) + x;
				if (idx >= 0u && idx < m_chunks.size() && !m_chunks[idx]->empty())
				{
					visible.push_back(m_chunks[idx].get());
				}
			}
		}

		std::swap(m_visibleChunks, visible);
	}

	// gets visible chunks of layer
	std::vector<MapLayer::Chunk*> getVisibleChunks(sf::RenderTarget& rt) {
		if (!m_isObjectLayer) {
			updateVisibility(rt.getView());

			return m_visibleChunks;
		}
	}

	// updates and return visible objects of layer
	std::vector<tmx::Object> getVisibleObjects(sf::View *view) {

		if (isObjectLayer()) {
			visibleObjects.clear();

			float vX = view->getCenter().x - view->getSize().x / 2;
			float vY = view->getCenter().y - view->getSize().y / 2;
			float vW = view->getSize().x;
			float vH = view->getSize().y;

			for (const auto& o : objects) {
				if (util::checkRectCollision(vX, vY, vW, vH, o.getAABB().left, o.getAABB().top, o.getAABB().width, o.getAABB().height))
					visibleObjects.push_back(o);
			}

			//std::cout << view->getCenter().x << std::endl;

			return visibleObjects;
		}
		else
			return visibleObjects;
	}

	void getVisibleTiles(const sf::View& view) const {
		const auto& layers = map->getLayers();
		const auto& layerProperties = layers[idx]->getProperties();
		const auto& tiles = layers[idx]->getLayerAs<tmx::TileLayer>().getTiles();
		//const auto& tileset = layers[idx]->getLayerAs<tmx::TileLayer>().get
		//std::cout << "wtf: " << tiles.at(0) << std::endl;
	}

	// updates list of visible chunkies
	void updateView(sf::View& view, sf::Vector2u windowSize, Player *player) const
	{
		std::vector<sf::RenderTexture> ctexs;
		sf::Vector2f viewCorner = view.getCenter();
		viewCorner -= view.getSize() / 1.8f;
		sf::Rect<float> viewRect(viewCorner.x, viewCorner.y, view.getSize().x*1.2f, view.getSize().y*1.2f);

		// updates camera;
		//sf::View tempView;
		//sf::Vector2f centeredPlayerPos(player->getPos().x + player->getCenterOffset().x,
		//	player->getPos().y + player->getCenterOffset().y);
		//tempView.setSize(view.getSize());
		//tempView.setCenter(centeredPlayerPos);
		//sf::Vector2f vpAnchor(view.getViewport().width * view.getCenter().x,
		//	view.getViewport().height * view.getCenter().y);
		this->camera.x = player->getPos().x - (windowSize.x * RES_FACTOR) * view.getViewport().width / 2;
		this->camera.y = player->getPos().y - (windowSize.y * RES_FACTOR) * view.getViewport().height /2;

		m_visibleChunkies.clear();
		int vCount = 0;
		for (int i = 0; i < m_chunkies.size(); i++) {
			m_chunkies.at(i).update(view, this->camera, *player);
			//sf::Rect<float> cRect(c., viewCorner.y, view.getSize().x, view.getSize().y);
			if (util::checkRectCollision(m_chunkies.at(i).rect, viewRect)) {
				vCount++;
				m_visibleChunkies.push_back(m_chunkies.at(i));
			}
		}

		this->player = player;
		//this->camera = sf::Vector2f(player->getPos().x, player->getPos().y);
		//std::cout << vCount << std::endl;
	}

	void draw(sf::RenderTarget& rt, sf::RenderStates states) const override
	{
		// draw tile layers
		if (!m_isObjectLayer) {
			//getVisibleTiles(rt.getView());
			//calc view coverage and draw nearest chunks
			//updateVisibility(rt.getView());
			//std::cout << "visChunksize: " << m_visibleChunks.size() << " | allChunkSize: " << m_chunks.size() << std::endl;
 		//	for (const auto& c : m_visibleChunks)
			//{
			//	// TODO
			//	// this is where tiles should be ordered before drawn!!
			//	// * put tiles in a ordered list of drawables 
			//	// ordered based on x and y !!
			//	rt.draw(*c, states);
			//}

			for (const auto& c : m_chunkies) {
				rt.draw(c, states);
				//util::drawRect(rt, c.rect, sf::Color::Black);
			}
			if (this->m_floor == player->floor) {
				player->draw(rt, camera);
			}
		}
		else if (DRAW_COLLIDERS) { // if is object layer draw colliders if option is set for debug
			auto color = m_layerType == LayerType::Collision ? sf::Color::Red : sf::Color::Green;
			for (const auto object : objects) {
				sf::Vector2f colScreen = sf::Vector2f(object.getAABB().left-camera.x,
														object.getAABB().top-camera.y);
				util::drawRect(rt, sf::Vector2f(colScreen.x, colScreen.y), 
						sf::Vector2f(colScreen.x + object.getAABB().width, colScreen.y),
						sf::Vector2f(colScreen.x + object.getAABB().width, colScreen.y + object.getAABB().height), 
						sf::Vector2f(colScreen.x, colScreen.y + object.getAABB().height),
						color);
			}
		}
	}
};

#endif //SFML_ORTHO_HPP_