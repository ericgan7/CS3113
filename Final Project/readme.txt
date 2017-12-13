
Controls:
	The game  is controlled solely by the mouse. I did not get the change to add keyboard shortcuts. You can hit escape to quit the while in game however.
	You can drag selected units, or single click units or buildings.
	The units have 4 states:
		Patrol, Attack, Move, and Attackmove

Gameplay:
	Resources update ever 8 hours.
	The goal is to kill all skeletons on the map & not have the command center destroyed.
	There possible things to build are:
	Cottage, Torch
	Farms
	Lumbermill, Quarry
	Wall, Gate, Ballista,
	Archers

There a three gamestates:
	Main menu
	Game
	End - end screen shows some stats from the game.

2 Shader programs:
	1 sets the color modifier as a uniform
	1 sets color per vertex (fog of war)

Known issues:
	Still update the camera with the projection matrix instead of the view matrix.
	I had a weird way of using the modelview matrix in hindsight.
	sampling along the points to see if there is impassible terrain does not work properly.
