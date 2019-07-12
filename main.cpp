//TODO:
	//Gamepad support
		//Test Joystick hats
	//Credits
		//Music for flood
	//Change default sound & music volumes
	//Mysterious temporary disappearances
		//Might be a glitch common to all actors
		//Physics shape stays locked in place
		//Actually, no, sometimes it turns
		//Sometimes when you touch a glitched enemy you get sucked into the void.
		//Might have something to do with collision shape changes and NaN parameters
		//The node is moving, but the body is not.
		//The model is shoved off somewhere far far away at first.
		//Has happened to: Player, Pyro Pastor, Postal Pope, Danger Deacon, Temp Templar
		//For Temp Templar, the ghost collision shape spins rapidly
		//Could be WeakChild issue?
		//For Pyro Pastor, root node is high in the sky while Rigidbody is on ground w/ very high upward velocity
	//Go through and make sure every class member is initialized
	//Enemies getting stuck in slopes
	//More posters?
	//Do final test through every level.
		//Make geometry shading flat
		//Make fog match the skybox color
		//FF4
			//Nerf the first section
			//Expand ridge in FF4
			//FF4 Enemy in the water?
			//Add bonus cross
			//Smoothen the gear interior
			//Bigger lifts to templar island
		//SS1
			//Reduce DD count
			//Add Bonus cross
			//Bad jump when returning from giant platform
			//Health pack under statue is too big
		//SS2
			//Make beginning platforms easier to cross
			//Can't get on platforms to return
			//Put checkpoint back at start
			//Lower that one PyP platform
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