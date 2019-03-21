//TODO:
	//Player disappears?
		//PP also disappears on SS1
	//Enemies out of range hang in midair
	//Indicator for "waiting" platforms?
	//Credits
		//Golden frames for concept art?
		//Rising "water"
		//Texturing
		//Optimizing
	//Extra Killer Kube Attack?
	//More posters?
	//Do final test through every level.
		//Make geometry shading flat
		//Make fog match the skybox color
		//PP1
			//Make inital ramp shorter
			//Make the giant cross a little less glitchy
		//PP3
			//Make platform in PP3 double-edged (and faster).
			//PP3 make that slope less steep
			//Danger Deacon gets stuck in floor on PP3
		//PP4
			//Add bonus cross
			//Nerf that spinning platform bit
		//FF1
			//FF1 is too ugly
			//Fallrhine 1 ledge is too high
		//FF2
			//Fallrhine 2 barrel platform unreachable
		//FF3
			//Exaggerate movement of spinning platform in FF3
			//FF3 health platform unreachable
			//Pillar unreachable
			//Faster platforms
			//Make broken bridge part less risky
			//More health
		//FF4
			//Expand ridge in FF4
			//FF4 Enemy in the water?
			//Add bonus cross
		//SS1
			//Reduce DD count
			//Add Bonus cross
			//Bad jump when returning from giant platform
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
			//Fix those tiny-ass platforms
	//Optimizations:
		//Optimize raycast distances for player (The more geometry that intersects it, the slower it gets)
		//Optimize physicsbodyquery?
		//Reduce rain particles on low graphics mode
		//Fix pause upon all enemies revive
		//Make explosions more efficient
	//Package assets
	
	//Send to Somecallmejohnny?
//Level Ideas
	//Giant cross platform rising up and down
	//Lifts sticking out of walls
	//Gigantic open area in center w/ bunch of lifts on edge heading towards small islands surrounding the center.
	//Make use of the beam material so that something's invisible until you revive.
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