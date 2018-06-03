//TODO:
	//Kaaba gets stuck
	//Replace cross on Tempest Templar, Pyro Pastor
	//Sinister Synagogue
		//Auto-adjusting antigravity platform component
	
	//Replace launchpad sound
	//Give Danger Deacons drowning animation
	//Replace snare drum in victory theme

	//Crossroads
		//Scrolling waluigi background
	//Credits
	//Palpan2
	//Palpan1

	//Sounds
		//Player death / Chime
		//God beam
		//Statue damage & destruction
		//Chaos caliph Charge & shoot
		//Killed Kaaba
			//Awakening
			//Bounce
			//Black hole noise
	//Bonus Crosses for: SinisterSynagogue, SS4, SS1, FF4, PP4, PP1, PP2. 
	//Do final test through every level.
		//Sacsand3 platforms going wrong way
		//Remove health from sacsand2
		//Fix exploit on sacsand1
		//Fallrhine 2 barrel platform unreachable

	//Look into SoundSounder optimization?
	//Make Enemy and Player derive from Actor instead of a has-a relationship?
	//Send E_SETTINGSCHANGE to self
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
	//HLSL's TEXCOORD(n) semantics are misleading

//COLLISION MASK
	//1 = ANYTHING THAT'S NOT THE MAP
	//2 = CAN BE JUMPED ON/SLOPED
	//4 = SHOOTABLE (STATUES)
	//8 = GROUND DETECTOR / TRIGGER
	//16 = SHIELD/BLACK HOLE
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