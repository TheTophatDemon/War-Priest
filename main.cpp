//TODO:
	//Sacsand4
		//Killer Kaaba
			//Black hole attack
	//Sinister Synagogue
		//Auto-adjusting antigravity platform component
	
	//Update missile warning texture
	//Remove specific religious symbols
	//Redo sandstone texture
	//Wololo
	//Give Danger Deacons drowning animation
	//Replace snare drum in victory theme
	//Replace sign post in FF4 with actual signpost model
	//Enhanced water gfx

	//Make Enemy and Player derive from Actor instead of a has-a relationship?
	//Send E_SETTINGSCHANGE to self

	//Crossroads
		//Scrolling waluigi background
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
	//Bonus Crosses for: SinisterSynagogue, SS4, SS1, FF4, PP4, PP1, PP2. 
	//Do final test through every level.
		//Sacsand3 platforms going wrong way
		//Remove health from sacsand2
		//Fix exploit on sacsand1
		//Fallrhine 2 barrel platform unreachable

	//Replace CompassScene with faster alternative?
	//Model-rigidbody lag?
	//Package assets
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
	//PrePhysicsUpdate problems?
	//Interpolation causes physics objects to jitter
	//Need an easier way of getting triangle info from a collision for "platforming logic"
	//Need a way to restrict transform inheritance from parents (rotation, location, scale)
	//The editor crashes sometimes when loading scenes for a second time
	//When the editor first opens, I am unable to edit any text until I alt-tab out and refocus the window.
	//When running fullscreen sometimes, after alt-tabbing the device gets lost over and over again, and the window flashes several times.
	//The editor keeps thinking "gamesettings.bin" is an XML file.
	//Custom components can't have parameters other than context
	//GetWorldTransform scaling bug?
	//Freezes on "SDL_Delay" (Called by DirectSound, I think) sometimes when re-focusing the window after an alt-tab.

//COLLISION MASK
	//1 = ANYTHING THAT'S NOT THE MAP
	//2 = CAN BE JUMPED ON/SLOPED
	//4 = SHOOTABLE (STATUES)
	//8 = GROUND DETECTOR / TRIGGER
	//16 = SHIELD/ONLY PROJECTILES CAN TOUCH
	//32 = ITEM
	//64 = ENEMY
	//128 = PLAYER
	//256 = LIQUID/WATER

//SOUND TYPES
	//GAMEPLAY
	//TITLE
	//MUSIC

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>

using namespace Urho3D;

#include "GunPriest.h"

URHO3D_DEFINE_APPLICATION_MAIN(GunPriest);