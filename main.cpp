//TODO:
	//Sounds
	//Postal Popes
			//Glowy crap
	//Make the water eat things
	//There should be, like, totally a car prop or something.
	
	//God damn motherfucking drop shadow
	//Autoload. Please.
	//Water splashing
	//Loading glitch?
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
	//64 = ENEMY
	//128 = PLAYER

//psychoruins

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>

using namespace Urho3D;

#include "GunPriest.h"

URHO3D_DEFINE_APPLICATION_MAIN(GunPriest);