//TODO:
	//Sounds
		//Player hurt
		//Play death
		//Play win & God's beam
		//HALLELUJAH!
	//Music Vol & Music

	//Enemy counter / Compass
	//Roblox?

	//Blood decal

	//Player clips through floor sometimes.
		//Likely a lift scaling issue
		//Have a pivot placeholder system

	//DelayedWorldTransform
	//Loading glitch/Alt-tab
		//Happens on clicking level select button...Might be a problem with Basic.hlsl
		//SDL_Delay bug
	
//Potential Optimizations:
	//Change raycast length
	//Less physics
	//Update event masks
	//Disable scripts on objects that are far away
	//Split map into multiple sections

//COLLISION MASK
	//1 = ANYTHING THAT'S NOT THE MAP
	//2 = CAN BE JUMPED ON/SLOPED
	//4 = NPC
	//8 = GROUND DETECTOR
	//16 = SHIELDS AND OTHER PROJECTILE BLOCKERS
	//32 = ITEM
	//64 = ENEMY
	//128 = PLAYER

//SOUND TYPES
	//GAMEPLAY
	//TITLE
	//MUSIC

//psychoruins

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>

using namespace Urho3D;

#include "GunPriest.h"

URHO3D_DEFINE_APPLICATION_MAIN(GunPriest);