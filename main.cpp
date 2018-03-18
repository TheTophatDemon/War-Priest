//TODO:
	//Sacsand4
		//Killer Kaaba
			//Gets stuck on lift
			//Attacking
				//Charging
				//Fireballs
					//Don't react with shield well enough
					//Change graphics. They's ugly.
	//Drop shadow goes through platforms
	//Add scrolling to level select menu

	//Sinister Synagogue
	//Crossroads
	//Credits
	//Palpan2
	//Palpan1

	//Sounds
		//Player death
		//God beam
		//HALLELUJAH!
		//Statue damage & destruction
		//Chaos caliph Charge & shoot
		//Bonus collection
		//Killed Kaaba
			//Awakening
			//Bounce
			//Charge?
	//Roblox?
	//Bonus Crosses for: SinisterSynagogue, SS4, SS1, FF4, PP4, PP1, PP2. 
	//Do final test through every level.
		//Remove health from sacsand2
		//Fix exploit on sacsand1

	//Model-rigidbody lag?
	//Package assets

	//Freezes on SDL_Delay called by DirectSound upon Alt-Tab
//Level Ideas
	//Giant cross platform rising up and down
	//Lifts sticking out of walls
	//Gigantic open area in center w/ bunch of lifts on edge heading towards small islands surrounding the center.
	//Make use of the beam material so that something's invisible until you revive.
	//Ascending a tower with a bunch of lifts
	//Ninja Nun?
		//Like danger deacon, but jumps alot and doesn't explode. Can maneuver around pits.
	//Radical Rabi?
		//Rides skateboard.

//Complaints:
	//It'd be nice to have an event that fires right before the physics steps for keeping track of flags
	//Interpolation causes physics objects to jitter
	//Need an easier way of getting triangle info from a collision for "platforming logic"
	//Need a way to restrict transform inheritance from parents (rotation, location, scale)
	//The editor crashes sometimes when loading other scenes
	//When the editor first opens, I am unable to edit any text until I alt-tab out and refocus the window.
	//When running fullscreen sometimes, after alt-tabbing the device gets lost over and over again, and the window flashes several times.
	//The editor keeps thinking "gamesettings.bin" is an XML file.
	//Custom components can't have parameters other than context

//COLLISION MASK
	//1 = ANYTHING THAT'S NOT THE MAP
	//2 = CAN BE JUMPED ON/SLOPED
	//4 = SHOOTABLE (STATUES)
	//8 = GROUND DETECTOR / TRIGGER
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