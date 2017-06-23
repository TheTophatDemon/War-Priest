//TODO:
	//Sounds
		//Player hurt
		//Play death
		//Play win & God's beam
		//HALLELUJAH!
		//Menu beeps and boops
	//Settings
		//Music Vol
		//Sound Vol
		//Low / High graphics
			//Texture filter
			//Material quality
			//Lighting mode
		//Controls configuration
		//Back button
		//Reset button
		//Mouse sensitivity
		//Cancel
		//Restart label
		//Blood settings (Replace with ultra gay sparkles)
		//Make sure settings are saved before exit
		
	//Could be because lack of update function
		//Fullscreen not working & Vsync
	//Correct settings not loaded at startup

	//Inline settings getters?
	//Clean up GunPriest.cpp
	//Exit button on title screen
		
	//Roblox?

	//Research unions

	//Loading glitch/Alt-tab
		//Happens on clicking level select button...Might be a problem with Basic.hlsl
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
	//GAMEPLAY
	//TITLE
	//MUSIC

//psychoruins

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>

using namespace Urho3D;

#include "GunPriest.h"

URHO3D_DEFINE_APPLICATION_MAIN(GunPriest);