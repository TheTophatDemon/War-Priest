//TODO:
	//Corpse finder
	//Winning
	//Level Select

	//Make enemies bigger
	
	//Material quality setup

	//Enemies overflowing AABB?
		//Usually happens when they spawn inside the map

	//Actually look up what Quaternions are
	
//Potential Optimizations:
	//Less physics
	//Lower physicsworld fps if possible
	//Disable UI subsystem?
	//Update event masks
	//Disable scripts on objects that are far away
	//Enemies far away = disable animation

//COLLISION MASK
	//1 = ANYTHING THAT'S NOT THE MAP
	//2 = CAN BE JUMPED ON/SLOPED
	//4 = NPC
	//8 = GROUND DETECTOR
	//64 = ENEMY
	//128 = PLAYER

//psychoruins

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>

using namespace Urho3D;

#include "GunPriest.h"

URHO3D_DEFINE_APPLICATION_MAIN(GunPriest);