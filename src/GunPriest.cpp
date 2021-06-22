/*
Copyright (C) 2021 Alexander Lunsford

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "GunPriest.h"
#include "Settings.h"
#include <Urho3D/IO/File.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <iostream>

#include "TempEffect.h"
#include "WeakChild.h"
#include "SoundSounder.h"
#include "Zeus.h"
#include "FlyCam.h"
#include "Actors/Player.h"
#include "Actors/Actor.h"
#include "Actors/Enemy.h"
#include "Actors/PyroPastor.h"
#include "Actors/ChaosCaliph.h"
#include "Actors/TempTemplar.h"
#include "Actors/TempShield.h"
#include "Actors/RobeLocksMan.h"
#include "Actors/God.h"
#include "Actors/KillerKube.h"
#include "Actors/BlackHole.h"
#include "Actors/PostalPope.h"
#include "Actors/DangerDeacon.h"
#include "Projectiles/Fireball.h"
#include "Projectiles/ProjectileWarner.h"
#include "Projectiles/RobeLocksMissile.h"
#include "Projectiles/RobeLocksBomb.h"
#include "Projectiles/Debris.h"
#include "Level/Lift.h"
#include "Level/Water.h"
#include "Level/CompassScene.h"
#include "Level/Launchpad.h"
#include "Level/Statue.h"
#include "Level/Bonus.h"
#include "Level/GravityPlate.h"
#include "Level/Sign.h"

int GunPriest::STATE_GAME = 0;
int GunPriest::STATE_TITLE = 1;

using namespace std;

GunPriest::GunPriest(Context* context) : Application(context)
{
	context_->RegisterSubsystem(new Script(context_));

	Gameplay::RegisterObject(context);
	Actor::RegisterObject(context);
	Player::RegisterObject(context);
	TempEffect::RegisterObject(context);
	TitleScreen::RegisterObject(context);
	PyroPastor::RegisterObject(context);
	Fireball::RegisterObject(context);
	God::RegisterObject(context);
	Lift::RegisterObject(context);
	PostalPope::RegisterObject(context);
	Debris::RegisterObject(context);
	Water::RegisterObject(context);
	WeakChild::RegisterObject(context);
	SoundSounder::RegisterObject(context);
	DangerDeacon::RegisterObject(context);
	Launchpad::RegisterObject(context);
	CompassScene::RegisterObject(context);
	TempTemplar::RegisterObject(context);
	Statue::RegisterObject(context);
	ChaosCaliph::RegisterObject(context);
	//Blackstone::RegisterObject(context);
	Bonus::RegisterObject(context);
	KillerKube::RegisterObject(context);
	//BouncyFireball::RegisterObject(context);
	ProjectileWarner::RegisterObject(context);
	TempShield::RegisterObject(context);
	GravityPlate::RegisterObject(context);
	GP::Sign::RegisterObject(context);
	Zeus::RegisterZeus(context);
	RobeLocksMan::RegisterObject(context);
	RobeLocksMissile::RegisterObject(context);
	BlackHole::RegisterObject(context);
	RobeLocksBomb::RegisterObject(context);
	FlyCam::RegisterObject(context);

	pause = false;
	state = STATE_TITLE;
}

void GunPriest::StartGame(String path)
{
	scene_->SetUpdateEnabled(false);
	game->ourUI->SetVisible(false);
	loadingScreen->SetVisible(true);
	cache->ReleaseResources(Model::GetTypeStatic(), true);
	cache->ReleaseResources(Material::GetTypeStatic(), true);
	cache->ReleaseResources(Texture2D::GetTypeStatic(), true);
	cache->ReleaseResources(XMLFile::GetTypeStatic(), true);

	engine_->RunFrame();
	std::cout << "RAN FRAME" << std::endl;

	scene_->SetUpdateEnabled(true);
	XMLFile* mapFile = cache->GetResource<XMLFile>(path);
	scene_->LoadXML(mapFile->GetRoot());
	scene_->AddComponent(game, 666, LOCAL);
	std::cout << "GAME ADDED" << std::endl;
	game->levelPath = path;
	game->SetupGame();
	std::cout << "GAME SETUP" << std::endl;

	scene_->SetElapsedTime(0.0f);

	loadingScreen->SetVisible(false);
	game->ourUI->SetVisible(true);

	debugRenderer = scene_->GetComponent<DebugRenderer>();
}

void GunPriest::Setup()
{
	engineParameters_["FullScreen"] = false;
	engineParameters_["VSync"] = false;
	engineParameters_["WindowWidth"] = 1280;
	engineParameters_["WindowHeight"] = 720;
	engineParameters_["WindowResizable"] = false;
	engineParameters_["WindowTitle"] = "War Priest";
	engineParameters_["Multisample"] = 0;
	engineParameters_["LogLevel"] = LOG_INFO | LOG_ERROR | LOG_WARNING;
}

void GunPriest::VideoSetup()
{
	Graphics* graphics = GetSubsystem<Graphics>();
	graphics->SetShaderCacheDir("Shaders/HLSL/Cache");
	const int refrate = graphics->GetRefreshRate();
	//Note that the engine may assign fullscreen resolutions that aren't among those listed in the Settings menu
	graphics->SetMode(Settings::GetResolutionX(), Settings::GetResolutionY(), Settings::IsFullScreen(), false, false, false, Settings::IsVsync(), false, 0, 0, refrate);
	renderer->SetDrawShadows(false);
	if (Settings::AreGraphicsFast())
	{
		renderer->SetTextureAnisotropy(0);
		renderer->SetTextureFilterMode(TextureFilterMode::FILTER_NEAREST);
		renderer->SetMaterialQuality(0);
		renderer->SetSpecularLighting(false);
	}
	else
	{
		renderer->SetTextureAnisotropy(1);
		renderer->SetTextureFilterMode(TextureFilterMode::FILTER_BILINEAR);
		renderer->SetMaterialQuality(2);
		renderer->SetSpecularLighting(true);
	}
}

void GunPriest::Start()
{
	SetRandomSeed(clock());
	engine_->SetMaxFps(60);

	input = SharedPtr<Input>(engine_->GetSubsystem<Input>());
	cache = GetSubsystem<ResourceCache>();
	ui = GetSubsystem<UI>();
	ui->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));
	renderer = GetSubsystem<Renderer>();
	audio = GetSubsystem<Audio>();

	Settings::LoadSettings(context_);
	VideoSetup();

	debugHud = engine_->CreateDebugHud();
#if _DEBUG
	debugHud->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DebugStyle.xml"));
	debugHud->SetMode(DEBUGHUD_SHOW_ALL);
#endif
	
	loadingScreen = ui->GetRoot()->LoadChildXML(cache->GetResource<XMLFile>("UI/loadingScreen.xml")->GetRoot());
	loadingScreen->SetVisible(false);

	/*SharedPtr<Text> text = SharedPtr<Text>(new Text(context_));
	text->SetText("WAR PRIEST ALPHA : WWW.BITENDOSOFTWARE.COM");
	text->SetFont("Fonts/Anonymous Pro.ttf", 24);
	text->SetHorizontalAlignment(HA_CENTER);
	text->SetVerticalAlignment(VA_TOP);
	text->SetPosition(0, 0);
	ui->GetRoot()->AddChild(text);*/

	viewport = new Viewport(context_);
	renderer->SetViewport(0, viewport);

	scene_ = new Scene(context_);

	game = new Gameplay(context_);
	game->gunPriest = this;

	titleScreen = new TitleScreen(context_);
	titleScreen->game = game;
	titleScreen->gunPriest = this;
	titleScreen->MakeMenus();
	scene_->AddComponent(titleScreen, 777, LOCAL);

	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(GunPriest, Update));
	SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(GunPriest, AfterRenderUpdate));
}

void GunPriest::ChangeState(int newState)
{
	if (newState == STATE_GAME && state == STATE_TITLE)
	{
		scene_->SetUpdateEnabled(true);
		titleScreen->ourUI->SetVisible(false);
		titleScreen->ourUI->SetEnabled(false);
		scene_->RemoveComponent(titleScreen);
		scene_->AddComponent(game, 666, LOCAL);
		scene_->SetElapsedTime(0.0f);
	}
	else if (newState == STATE_TITLE && state == STATE_GAME)
	{
		titleScreen->SetMenu(titleScreen->titleMenu);
		scene_->SetUpdateEnabled(false);
		game->ourUI->SetVisible(false);
		game->ourUI->SetEnabled(false);
		scene_->RemoveComponent(game);
		scene_->AddComponent(titleScreen, 777, LOCAL);
		scene_->SetElapsedTime(0.0f);
	}
	state = newState;
}

void GunPriest::Update(StringHash eventType, VariantMap& eventData)
{
	if (state == STATE_GAME)
	{
		if (input->GetKeyDown(KEY_KP_MULTIPLY))
		{
#if _DEBUG
			pause = true;
			if (input->GetKeyPress(KEY_KP_MINUS))
			{
				pause = false;
			}
#endif
		}
		else
		{
			pause = false;
		}
		scene_->SetUpdateEnabled(!pause);
		if (Settings::IsActionPressed(ActionType::MENU))
		{
			ChangeState(STATE_TITLE);
		}
	}
}

void GunPriest::AfterRenderUpdate(StringHash eventType, VariantMap& eventData)
{
	if (state == STATE_GAME && game->initialized)
	{

		if (input->GetKeyDown(KEY_KP_DIVIDE)) scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(debugRenderer, true);

	}
}

void GunPriest::Stop()
{
	
}
