//TODO:
	//Sounds
	//Healthpacks
	//Make player return to start
	
	//Loading glitch/Alt-tab
	//Material quality setup
	
//Potential Optimizations:
	//Change raycast length
	//Less physics
	//Update event masks
	//Disable scripts on objects that are far away

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
	//ENVIRONMENT
	//VOICE

//psychoruins

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>

using namespace Urho3D;

#include "GunPriest.h"

URHO3D_DEFINE_APPLICATION_MAIN(GunPriest);