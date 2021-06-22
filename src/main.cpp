//TODO:
	//PP3 make start platform more accessible
	//More posters?
	//Tiny glitches
		//Enemies getting stuck in slopes
			//Always seems to be the pyro pastors
		//Boulders can roll out of water and disappear suddenly
		//Postal Pope boulders being shot into walls
		//Chaoscaliph not drowning in SS4?
			//Splash particles are going
		//Postal pope rising into ceilings
		//Player disappeared in win sequence?
		//Fix pause upon all enemies revive
			//Doesn't happen on SS2??
	//Optimize raycast distances for player (The more geometry that intersects it, the slower it gets)
	//Optimize physicsbodyquery?
	//Package assets?
	//Send to Youtubers?
//Complaints:
	//PrePhysicsUpdate problems?
	//Interpolation causes physics objects to jitter
	//Need an easier way of getting triangle info from a collision for "platforming logic"
	//Need a way to restrict transform inheritance from parents (rotation, location, scale)
	//The editor crashes sometimes when loading scenes for a second time
	//When the editor first opens, I am unable to edit any text until I alt-tab out and refocus the window.
	//When running fullscreen sometimes, after alt-tabbing the device gets lost over and over again, and the window flashes several times.
	//Can't stop the terrain editor
	//The editor keeps thinking "gamesettings.bin" is an XML file.
	//The controls near the top of the Hierarchy window in the Editor are confusing
	//Can't unparent selections that are selected using "shift"
	//Custom components can't have parameters other than context
	//Trouble with Text3D custom materials
	//Freezes on "SDL_Delay" (Called by DirectSound, I think) sometimes when re-focusing the window after an alt-tab.
	//HLSL's TEXCOORD(n) semantics are misleading
	//Text UI not saving size?
	//No documentation for:
		//CreateTemporaryChild
	//Particle effects don't turn on until you see them, despite being set to emit
	//Editor doesn't duplicate hierarchies correctly when multiple items selected
	//Raycasts can be extremely slow in debug mode when going through lots of geometry
	//Extreme lag when scaling kinematic rigidbody
	//Blender export blurs vertex colors together
	//Value animations reset when wrapping mode is changed.
	//UI elements send events even when they are removed or their parent is disabled?
//COLLISION MASK
	//1 = ANYTHING THAT'S NOT THE MAP
	//2 = CAN BE JUMPED ON/SLOPED
	//4 = SHOOTABLE (STATUES AND PLAYERS)
	//8 = GROUND DETECTOR / TRIGGER
	//16 = SHIELD/BLACK HOLE
	//32 = ITEM
	//64 = ENEMY
	//128 = PLAYER
	//256 = LIQUID/WATER
	//512 = THE MAP ONLY

//SOUND TYPES
	//GAMEPLAY
	//TITLE
	//MUSIC

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/Application.h>

using namespace Urho3D;

#include "GunPriest.h"

URHO3D_DEFINE_APPLICATION_MAIN(GunPriest);