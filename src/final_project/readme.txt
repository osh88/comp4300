COMP 4300 - Final Project Specification (Fall 2022)

The final course project is to complete a 'full game' using the specification in this document. The game must have
the bare minimum functionality mentioned here, and any extra functionality will be greatly appreciated. Projects
will be done in groups of UP TO 4 people. Groups of exactly 4 people are greatly preferred. It must be 'significantly
different' from existing assignments.

Note: This document is very long because it's better to give too many details than to leave them out.

Project Marking and Due Date Overview
1. Game Proposal            05% Due November 15th, 2022 @ 11:59pm
2. Initial Video Demo       10% Due December 8th,  2022 @ 11:59pm
3. Final Project Code       60% Due December 15th, 2022 @ 11:59pm
4. Final Presentation Video 20% Due December 15th, 2022 @ 11:59pm
5. Final YouTube Trailer    05% Due December 15th, 2022 @ 11:59pm

Project GitHub Template
For this project you will use the following GitHub template repo: https://github.com/davechurchill/COMP4300 2022 Project
Directions on how to use this template repo will be discussed in the project lecture.

Project Proposal (5%)
    Your group must submit a project proposal detailing what you plan to do with your group. Your proposal will be
    reviewed to determine whether it is 'enough work' or too much work' for the scope of the course. The project
    proposal should be a single PDF file uploaded to the Proposal folder on GitHub, describing the following
    information, and be approximately 2-3 pages:
    - Game name / genre / overall theme / main gameplay style
    - Examples of gameplay scenarios involving each of the game mechanics described below
    - A list of 'extra' features of the game which have not yet been completed in assignments

Project Demo (10%)
    3 weeks after the project proposal, and 1 week before the final due date of the project, you are required to submit
    a video approximately 3 minutes in length which shows off the features of the game which you have gotten to work so
    far. You do not have to have all the functionality of the game working, but you must have a functioning game with
    its main gameplay at this point. This milestone is in place to ensure you keep on track for your final submission.
    Please show off all the working functionality of the game with short voice-over commentary on what is working and
    what is not working at this point.

Final Project Required Game Features / Gameplay (60%)

1. Game Overview
    - Game must be implemented using ECS architecture in C++ using only the SFML library
    - You may use any course code already written as the basis for your game
    - Game Types: 2D Platformer, Top-Down Shooter, RPG, Action Adventure, etc
    - Must contain at least 3 pre-built levels, and have a 'final boss' battle
    - Must contain a custom menu that allows the player to play the game, edit levels, or select options
    - Must contain some sort of in-game menu (item selection, inventory, options, etc)
    - Must contain a level editor that allows for loading, editing, and saving of game levels (see relevant section)
    - Must contain a 'game over' screen indicating when the game has been finished
    - All levels, player, and game configuration options must be defined in external text files
    - All assets should be gathered or created by the project group members

2. Game Scenes
    - Must contain a main menu scene that implements the main menu functionality
    - Must contain an overworld map scene that allows for level selection / game progression (Super Mario World etc)
    - Must contain a main gameplay scene that implements the game physics of the main gameplay mode
    - Must contain some sort of item inventory or in-game many scene that is used for a relevant function
    - Must contain a level editor scene that implements level editor functionality
    - Must contain a 'game over' scene with some sort of animation and game over / credits

3. Required Gameplay and Mechanics
    Your game must contain all the following mechanics. Describe how they will be used in the proposal.
    - Collisions - Your game must include rectangular bounding box collisions between some entities
    - Bullets / Weapons - Your game must include MULTIPLE weapons that are usable by the player and swappable during
      gameplay. For example, Mega Man or Link swapping weapons during play.
    - NPCs - There must be non-player characters in the game that act as enemies / allies for certain gameplay elements.
      Some of these NPCs must contain basic Al such as path-finding / shooting / patrolling / battling with the player.
    - Moving Tiles - Your game must include some part of the level which moves, such as platforms / elevators
    - HP / Damage - Player/enemies in the game should have hit points (life) and take damage / die
    - Status Effects - Your game must contain at least 3 separate status effects, which the player can obtain in some
      way (item, collision, ability, etc) which alter the gameplay for a limited amount of time. For example: speed
      potion makes the player run faster, temporary invulnerability like a Super Mario star, or a Quad Damage like
      item from Quake
    - Objects / Inventory - The player should have an inventory of items which can be picked up during play and used
      For example: Health packs / sprint potions. An in-game menu must show the available items of the player somehow
    - Ray Casting - Visibility / ray casting calculations should be used in some form in the game. For example: the
      enemy could not react until they see the player (turret, etc)
    - Lighting Effects - Your game must contain some sort of lighting effect, similar to the one demonstrated in class
      via the "Sight and Light demo. For example: carrying a torch/flashlight around a level, or placing light sources
      within a level.
    - Gravity / Acceleration - There must be some form of gravity / attractor in the game that applies acceleration
      to the player
    - Camera / World View - Your game must use at least 2 different camera views in an interesting way
    - Pathfinding/Steering - Some entities in the game must exhibit non-trivial pathfinding and smooth steering behaviour
    - Game Progression - Your overworld map should somehow lock or unlock game progression based on levels completed
    - Save/Load Game - You must have the ability to save and load your game progress somehow to a file
    - Shaders - Some entities in the game must use shaders that alter their appearance in some meaningful way
    - Parallax - You must incorporate parallax via multiple background layers in some way in your game
    - User Interface / HUD - Your game must have a user interface / HUD which displays information such as player health,
      ammo, game progression, status effects, NPC life bars, etc
    - Sounds - Your game must have music that plays in the background, as well as unique sounds associated with attacking,
      taking or dealing damage, killing enemies, picking up items, finishing levels, etc
    - Options - Your game must have an options menu which allows you to change the following settings:
      - Music Volume, Sound Effects Volume (separate options)
      - Game Difficulty - Normal, Easy (deal 2x, take 0.5x damage), Hard (deal 0.5x, take 2x damage)
      - You can use a global Difficulty namespace to store variables related to these settings
      - Rebind main gameplay scene keys - for example moving left / right, jump, shoot, etc
    - Extras - 10% of the mark of the project is reserved for extra / new mechanics not specifically listed here.
      Examples of this include really cool special effects, complicated weapons, very polished user interface, etc

4. Level Editor (examples: Mega Man Maker / Super Mario Maker)
    - Your game must contain a level editor similar to the one found in the Mega ManMaker / Super Mario Maker game
    - All of your main gameplay levels must be able to be made from within the editor. Overworld map can be hard-coded
    - This level editor should operate on a grid, similar to the one found in Assignment 3 / 4
    - The level editor should have a menu that allows you to select an existing level to edit (sample code on my GitHub:
      https://github.com/davechurchill/stardraft/blob/master/src/GameState_Menu.cpp)
    - You should be able to select and place any Texture / Animation defined in the Assets file into the level
    - Any parameters of specific entities must be editable via the level editor. For example, this can include:
      - Whether NPCs block vision, movement, or neither
      - The hit points / damage of NPCs in the level
      - The patrol points of moving tiles or NPCs
    - How you accomplish parameter input is completely up to you. One way may to build menu that pops up. Or you could
      select a parameter via a key press and then edit it up or down via the mouse wheel. I don't care exactly how you
      do it, as long as you explain how to use it in the report.

5. Game Assets
    - Your game cannot re-use any assets that have been given out in the course (besides mega man / link sprite)
    - You do not need to create original game assets from scratch, but you do need to obtain them from somewhere

YouTube Game Trailer (5%)
    - 5% of the project mark will be for making a YouTube video trailer for your game (2-3 minutes long)
    - Sample trailers from previous course offerings can be seen on my teaching website
    - Trailers will be listed on the course website for future classes to see, and you can also add them to your portfolio

Project Report Video (20%)
    Your project must contain a video report / presentation (approx 10-15 minutes) containing the following information:
    - An overview of the game giving all details described in 'Game Overview' above, including asset sources
    - A description of all 'extra features' put into the game on top of those specifically asked for
    - Demonstrations of all game mechanics, with at least one full level playthrough. Also demo the level editor.
    - Detailed list of controls for the game, and instructions on how to play / what the objectives are
    - Notes on anything you tried to implement that did not end up working
    - This video must contain audio commentary / explanation, and be uploaded to YouTube

Game Ideas
    - If you are absolutely stuck for a game idea, I would recommend checking out some older NES/SNES games for ideas
    - For example, if you want to recreate the mechanics of Super Mario 3 or Mega Man 2, that could work. Just be sure
      to implement all of the required project features (that may not be present in those games) as well.

