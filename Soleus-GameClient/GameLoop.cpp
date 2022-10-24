#include <SFML/System.hpp>
#include <thread>
#include <chrono>
#include <iostream>
#include "Game.h"


//====================
// Constructors
//====================
GameLoop::GameLoop() { localTime = 0.f; state = GameState::Startup; }

//====================
// Gameloop execution
//====================
void GameLoop::run() {
	// loads configuration before game loop starts
	if (!load()) { // if something goes wrong, enters shutdown state
		state = GameState::Shutdown; 
	}

	// Clock used to get delta time
	sf::Clock clock;

	// runs gameloop until shutdown state
	while (GameState::Shutdown != state) {
		// Update the game then draw
		float dt = clock.restart().asSeconds();
		update(dt);
		draw();
	}

	// calls shutdown after gameloop has ended
	shutdown();
}

void GameLoop::run(float step) {
	if (!load()) {
		state = GameState::Shutdown;
	}

	// Fixed time-step
	sf::Clock clock;
	const float dt = 1.0f / step;
	float accumulator = 0.0f;

	while (GameState::Shutdown != state) {
		// Update the game then draw
		float frameTime = clock.restart().asSeconds();

		if (frameTime > 0.25f) {
			frameTime = 0.25f;
		}

		accumulator += frameTime;
		localTime += frameTime;

		while (accumulator >= dt) {
			update(dt);
			accumulator -= dt;
		}

		draw();

		// Prevent CPU from killing itself
		long timeLeft = static_cast<long>(1000 * dt - clock.restart().asMilliseconds());
		if (timeLeft > 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(timeLeft));
		}
	}

	shutdown();
}
