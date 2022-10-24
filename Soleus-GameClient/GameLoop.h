#ifndef GAMELOOP_H
#define GAMELOOP_H

#include <SFML\System\Clock.hpp>
#include <chrono>
#include <thread>

namespace GameState
{
	/// Values that represent the possible GameStates.
	enum
	{
		Startup,
		WaitingForClients,
		Preparatory,
		Running,
		Shutdown
	};
}

/// An Abstract Class that structures the game loop
/// It controls the flow and implementation should contain core functionalities
class GameLoop
{
public:
	int state;          ///< The current state of the Game
	float localTime;    ///< Local seconds running

	GameLoop();
	void run();						///< Runs game loop 
	void run(float fixedFps);		///< Runs game loop at a fixed timestep

	//====================
	// Interface
	//====================

	/// Called after construction, immediately before entering the update loop.
	/// Connection establishment and resource loading should go here. If this
	/// method returns false, the Game will not enter the update loop, and it
	/// will immediately switch to the Shutdown state.
	virtual bool load() = 0;

	/// Called after the update loop has finished. Frees any resources used by
	/// the game before exiting.
	virtual void shutdown() = 0;

	/// Called on every tick. Because we're running at a fixed time-step, this
	/// should be called at a regular interval.
	/// @param    dt    The time since the last update (seconds).
	virtual void update(float dt) = 0;

	/// Draw the Game
	virtual void draw() { }
};

#endif