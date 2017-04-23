#include <string>
#include <iostream>

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/Physics/PhysicsWorld.h>

#include <time.h>

#include "Gameplay.h"
#include "Player.h"
#include "Actor.h"
#include "TempEffect.h"
#include "Boulder.h"
#include "Enemy.h"
#include "TitleScreen.h"
#include "PyroPastor.h"
#include "Projectile.h"

//TODO:
	//Pyro Pastor model + animation
	//Sliding
		//Add limitations
		//Invincibility
	//Player revive effects
	
	//Pillar LODS
	//Broken ceiling collisions

	//Enemies overflowing AABB?
		//Usually happens when they spawn inside the map

	//Actually look up what Quaternions are
	
//Potential Optimizations:
	//Less physics
	//Lower physicsworld fps if possible
	//Disable UI subsystem?
	//Update event masks
	//Disable scripts on objects that are far away

//COLLISION MASK
	//1 = ANYTHING THAT'S NOT THE MAP
	//2 = CAN BE JUMPED ON/SLOPED
	//4 = NPC
	//8 = GROUND DETECTOR
	//64 = ENEMY
	//128 = PLAYER

//psychoruins

using namespace Urho3D;

#define STATE_GAME 0
#define STATE_TITLE 1

class GunPriest : public Application
{
public:
	SharedPtr<Scene> scene_;
	SharedPtr<Renderer> renderer;
	SharedPtr<Input> input;
	SharedPtr<ResourceCache> cache;
	SharedPtr<UI> ui;

	SharedPtr<Gameplay> game;
	SharedPtr<TitleScreen> titleScreen;

	SharedPtr<Viewport> viewport;
	SharedPtr<DebugHud> debugHud;
	SharedPtr<DebugRenderer> debugRenderer;
	SharedPtr<Text> loadingText;

	int state = STATE_TITLE;
	GunPriest(Context* context) : Application(context)
	{
		Gameplay::RegisterObject(context);
		Actor::RegisterObject(context);
		Player::RegisterObject(context);
		TempEffect::RegisterObject(context);
		Boulder::RegisterObject(context);
		TitleScreen::RegisterObject(context);
		Enemy::RegisterObject(context);
		PyroPastor::RegisterObject(context);
		Projectile::RegisterObject(context);
	}
	void StartGame()
	{
		game->ourUI->SetVisible(false);
		loadingText->SetVisible(true);
		cache->ReleaseAllResources(false);

		engine_->RunFrame();
		XMLFile* mapFile = cache->GetResource<XMLFile>("Scenes/palisadepantheon.xml");
		scene_->LoadXML(mapFile->GetRoot());
		scene_->AddComponent(game, 666, LOCAL);
		game->SetupGame();

		loadingText->SetVisible(false);
		game->ourUI->SetVisible(true);

		debugRenderer = scene_->GetComponent<DebugRenderer>();
	}
	void SetupRenderer()
	{
		renderer->SetDrawShadows(false);
		renderer->SetTextureAnisotropy(0);
	}
	virtual void Setup()
	{
		engineParameters_["FullScreen"] = false;
		engineParameters_["WindowWidth"] = 1280;
		engineParameters_["WindowHeight"] = 720;
		engineParameters_["WindowResizable"] = false;
		engineParameters_["Multisample"] = 0;
		context_->RegisterSubsystem(new Script(context_));
	}
	virtual void Start()
	{
		SetRandomSeed(clock());
		input = SharedPtr<Input>(engine_->GetSubsystem<Input>());
		engine_->SetMaxFps(60);
		cache = GetSubsystem<ResourceCache>();
		ui = GetSubsystem<UI>();
		ui->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));
		renderer = GetSubsystem<Renderer>();

		engine_->SetGlobalVar("SCREEN WIDTH", engineParameters_["WindowWidth"]);
		engine_->SetGlobalVar("SCREEN HEIGHT", engineParameters_["WindowHeight"]);
		SetupRenderer();
		
		debugHud = engine_->CreateDebugHud();
#if _DEBUG
		debugHud->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));
		debugHud->SetMode(DEBUGHUD_SHOW_ALL);
#endif
		loadingText = new Text(context_);
		loadingText->SetText("LOADING...");
		loadingText->SetFont("Fonts/Anonymous Pro.ttf", 48);
		loadingText->SetHorizontalAlignment(HA_CENTER);
		loadingText->SetVerticalAlignment(VA_CENTER);
		loadingText->SetPosition(0,-24);
		ui->GetRoot()->AddChild(loadingText);
		loadingText->SetVisible(false);

		viewport = new Viewport(context_);
		renderer->SetViewport(0, viewport);

		scene_ = new Scene(context_);
		game = new Gameplay(context_);
		titleScreen = new TitleScreen(context_);
		titleScreen->game = game;

		scene_->AddComponent(titleScreen, 777, LOCAL);
		SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(GunPriest, Update));
		SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(GunPriest, AfterRenderUpdate));
	}
	void ChangeState(int newState) //Handles transitions between states
	{
		if (newState == STATE_GAME && state == STATE_TITLE)
		{
			scene_->SetUpdateEnabled(true);
			titleScreen->ourUI->SetVisible(false);
			titleScreen->ourUI->SetEnabled(false);
			scene_->RemoveComponent(titleScreen);
			scene_->AddComponent(game, 666, LOCAL);
		}
		else if (newState == STATE_TITLE && state == STATE_GAME)
		{
			scene_->SetUpdateEnabled(false);
			game->ourUI->SetVisible(false);
			game->ourUI->SetEnabled(false);
			scene_->RemoveComponent(game);
			scene_->AddComponent(titleScreen, 777, LOCAL);
		}
		state = newState;
	}
	void Update(StringHash eventType, VariantMap& eventData)
	{
		if (state == STATE_GAME)
		{
			if (game->loseTimer == 1)
			{
				game->loseTimer = 0;
				StartGame();
			}
			if (input->GetKeyPress(KEY_ESCAPE))
			{
				ChangeState(STATE_TITLE);
			}
		}
		else if (state == STATE_TITLE) 
		{
			if (input->GetKeyPress(KEY_RETURN) || titleScreen->gotoGame)
			{
				ChangeState(STATE_GAME);
				if (!game->initialized)
				{
					StartGame();
				}
			}
			if (input->GetKeyPress(KEY_ESCAPE) && titleScreen->IsEnabled())
			{
				engine_->Exit();
				return;
			}
		}
	}
	void AfterRenderUpdate(StringHash eventType, VariantMap& eventData)
	{
		if (state == STATE_GAME && game->initialized)
		{
			scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(debugRenderer, true);
		}
	}
	virtual void Stop()
	{
		
	}
};URHO3D_DEFINE_APPLICATION_MAIN(GunPriest);