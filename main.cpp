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

#include <time.h>

#include "Gameplay.h"
#include "Player.h"
#include "Cross.h"
#include "NPC.h"
#include "Actor.h"
#include "TempEffect.h"
#include "Boulder.h"
#include "TitleScreen.h"

//TODO:
	//Compass
	//Door
	//Lose condition
	//Winning
	//Death (possibly by lava)
	//Level select screen
	//Bug: Npc's getvar voice not returning

//Potential Optimizations:
	//Less physics
	//Lower physicsworld fps if possible
	//Disable UI subsystem?
	//Update event masks

using namespace Urho3D;

class GunPriest : public Application
{
public:
	SharedPtr<Scene> scene_;
	SharedPtr<Renderer> renderer;
	SharedPtr<Input> input;
	SharedPtr<ResourceCache> cache;

	SharedPtr<Gameplay> game;
	SharedPtr<TitleScreen> titleScreen;

	SharedPtr<DebugHud> debugHud;
	SharedPtr<DebugRenderer> debugRenderer;
	SharedPtr<Text> loadingText;

	
	GunPriest(Context* context) : Application(context)
	{
		Gameplay::RegisterObject(context);
		Actor::RegisterObject(context);
		Player::RegisterObject(context);
		Cross::RegisterObject(context);
		NPC::RegisterObject(context);
		TempEffect::RegisterObject(context);
		Boulder::RegisterObject(context);
		TitleScreen::RegisterObject(context);
	}
	void StartGame()
	{
		loadingText->SetVisible(true);
		engine_->RunFrame();
		XMLFile* mapFile = cache->GetResource<XMLFile>("Scenes/map02.xml");
		scene_->LoadXML(mapFile->GetRoot());
		game = scene_->CreateComponent<Gameplay>();
		game->SetupGame();
		game->SetEnabled(false);
		loadingText->SetVisible(false);
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
		UI* ui = GetSubsystem<UI>();
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

		scene_ = new Scene(context_);
		StartGame();
		
		titleScreen = scene_->CreateComponent<TitleScreen>();

		debugRenderer = scene_->GetComponent<DebugRenderer>();
		SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(GunPriest, Update));
	}
	void Update(StringHash eventType, VariantMap& eventData)
	{
		if (input->GetKeyPress(KEY_RETURN) && !game->IsEnabled())
		{
			scene_->SetUpdateEnabled(true);
			game->SetEnabled(true);
			titleScreen->SetEnabled(false);
		}
		else if (input->GetKeyPress(KEY_ESCAPE) && game->IsEnabled())
		{
			scene_->SetUpdateEnabled(false);
			game->SetEnabled(false);
			titleScreen->SetEnabled(true);
		}
		else if (input->GetKeyPress(KEY_ESCAPE) && titleScreen->IsEnabled())
		{
			engine_->Exit();
			return;
		}
	}
	virtual void Stop()
	{
		
	}
};URHO3D_DEFINE_APPLICATION_MAIN(GunPriest);