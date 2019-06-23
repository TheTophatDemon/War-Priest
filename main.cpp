//TODO:
	//Player disappears?
		//PP also disappears on SS1
		//First person mode makes the model pop out, but it's stuck in position
	//Gamepad support
		//Test Joystick hats
	//Credits
		//Music for flood
	//More posters?
	//Do final test through every level.
		//Make geometry shading flat
		//Make fog match the skybox color
		//FF2
			//Fallrhine 2 barrel platform unreachable
		//FF3
			//Exaggerate movement of spinning platform in FF3
			//FF3 health platform unreachable
			//Pillar unreachable
			//Faster platforms
			//Make broken bridge part less risky
			//Big jump from blue-tipped tower is too larges
			//More health
		//FF4
			//Nerf the first section
			//Expand ridge in FF4
			//FF4 Enemy in the water?
			//Add bonus cross
		//SS1
			//Reduce DD count
			//Add Bonus cross
			//Bad jump when returning from giant platform
			//Health pack under statue is too big
		//SS2
			//Make beginning platforms easier to cross
			//Can't get on platforms to return
			//Put checkpoint back at start
			//Lower that one PP platform
		//SS3
			//Sacsand3 platforms going wrong way
			//SS3 less enemies on side island
		//SS4
			//SS4 intial launchpad comes up too short
			//Add a way back from that initial launchpad
			//Fix those tiny-ass platforms
	//Optimizations:
		//Optimize raycast distances for player (The more geometry that intersects it, the slower it gets)
		//Optimize physicsbodyquery?
		//Fix pause upon all enemies revive
	//Package assets
	
	//Send to Somecallmejohnny?

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

using namespace Urho3D;

#include "GunPriest.h"

URHO3D_DEFINE_APPLICATION_MAIN(GunPriest);