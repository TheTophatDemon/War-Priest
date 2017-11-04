//TODO:
	//Sacsand 2

	//Crazy Caliph
		//Shoots a physics projectile that turns into a homing missile after a few seconds.

	//Portals?
	
	//Sounds
		//Player death
		//God beam
		//HALLELUJAH!
		//Statue damage & destruction
	//Roblox?
	//LODs for: TT, DD, and PostalPope
	//Package assets

	//Freezes on SDL_Delay called by DirectSound upon Alt-Tab

//Level Ideas
	//Two PP's and a TT
	//Have a PP way below you and firing upwards
	//Little protrusions from the ground that you hop on before descending to the lower levels
	//Ascending a tower with a bunch of lifts
	
//Potential Optimizations:
	//Change raycast length
	//Less physics
	//Update event masks
	//Disable scripts on objects that are far away
	//Split map into multiple sections

//Complaints:
	//It'd be nice to have an event that fires right before the physics steps for keeping track of flags
	//Annoying physics jitter due to interpolation
	//Need an easier way of getting triangle info from a collision for "platforming logic"
	//Need a way to restrict transform inheritance from parents (rotation, location, scale)
	//The editor crashes sometimes when loading other scenes
	//When the editor first opens, I am unable to edit any text until I alt-tab out and refocus the window.
	//When running fullscreen, after alt-tabbing the device gets lost over and over again, and the window flashes several times.
	//The editor keeps thinking "gamesettings.bin" is an XML file.

//COLLISION MASK
	//1 = ANYTHING THAT'S NOT THE MAP
	//2 = CAN BE JUMPED ON/SLOPED
	//4 = SHOOTABLE
	//8 = GROUND DETECTOR
	//16 = SHIELD/ONLY PROJECTILES CAN TOUCH
	//32 = ITEM
	//64 = ENEMY
	//128 = PLAYER

//SOUND TYPES
	//GAMEPLAY
	//TITLE
	//MUSIC

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>

using namespace Urho3D;

#include "GunPriest.h"

URHO3D_DEFINE_APPLICATION_MAIN(GunPriest);