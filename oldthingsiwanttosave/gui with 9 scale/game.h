/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Dog.h"
#include "Enemy.h"
#include "TileMap.h"
#include "LevelManager.h"
#include "PauseMenu.h"

#include "NuklearRenderer.h" // Forward-declared or included
#include <nuklear.h>

#include "NineSliceRenderer.h"

// Represents the current state of the game
enum GameState {
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN
};

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Game {
public:
	Game(unsigned int width, unsigned int height);
	~Game();

	void Init();
	void ProcessInput(GLFWwindow* window, float dt);
	void Update(float dt);
	void Render();
	void SetSize(unsigned int width, unsigned int height);

	void HandlePauseMenuSelection(PauseMenu::Option opt, GLFWwindow *window);


	GameState State;
	bool Keys[1024];
	unsigned int Width, Height;

	void RenderUI();  // new
	void SetUIRenderer(NuklearRenderer* gui);
	NuklearRenderer* GUI = nullptr;
	NineSliceRenderer* panelRenderer = nullptr;
	NineSliceRenderer* continueRenderer = nullptr;
	NineSliceRenderer* continuehotRenderer = nullptr;
	NineSliceRenderer* levelSelectRenderer = nullptr;
	NineSliceRenderer* levelSelectHotRenderer = nullptr;
	NineSliceRenderer* quitRenderer = nullptr;
	NineSliceRenderer* quitHotRenderer = nullptr;


private:
	TileMap* TileMap_;
	Dog* dog_;
	Enemy* slime1_;
	Enemy* skeleton1_;
	LevelManager levelManager_;
};

#endif
