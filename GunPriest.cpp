#include "GunPriest.h"
#include "Settings.h"
#include <Urho3D/IO/File.h>
#include <iostream>

int GunPriest::STATE_GAME = 0;
int GunPriest::STATE_TITLE = 1;

using namespace std;

GunPriest::GunPriest(Context* context) : Application(context)
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
	God::RegisterObject(context);
	Lift::RegisterObject(context);
	PostalPope::RegisterObject(context);
	Debris::RegisterObject(context);
	Water::RegisterObject(context);
	WeakChild::RegisterObject(context);
	SoundSounder::RegisterObject(context);
	DangerDeacon::RegisterObject(context);

	state = STATE_TITLE;
	sSoundVolume = 0.5f;
}

void GunPriest::StartGame(String path)
{
	game->ourUI->SetVisible(false);
	loadingText->SetVisible(true);
	cache->ReleaseAllResources(true);

	engine_->RunFrame();
	std::cout << "RAN FRAME" << std::endl;
	XMLFile* mapFile = cache->GetResource<XMLFile>(path);
	scene_->LoadXML(mapFile->GetRoot());
	scene_->AddComponent(game, 666, LOCAL);
	std::cout << "GAME ADDED" << std::endl;
	game->SetupGame();
	std::cout << "GAME SETUP" << std::endl;

	loadingText->SetVisible(false);
	game->ourUI->SetVisible(true);

	debugRenderer = scene_->GetComponent<DebugRenderer>();
}

void GunPriest::Setup()
{
	Settings::LoadSettings(context_);
	engineParameters_["FullScreen"] = Settings::IsFullScreen();
	engineParameters_["VSync"] = Settings::IsVsync();
	engineParameters_["WindowWidth"] = 1280;
	engineParameters_["WindowHeight"] = 720;
	engineParameters_["WindowResizable"] = false;
	engineParameters_["WindowTitle"] = "War Priest";
	engineParameters_["Multisample"] = 0;
	context_->RegisterSubsystem(new Script(context_));
}

void GunPriest::VideoSetup()
{
	renderer->SetDrawShadows(false);
	renderer->SetTextureAnisotropy(0);
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

	VideoSetup();

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
	loadingText->SetPosition(0, -24);
	ui->GetRoot()->AddChild(loadingText);
	loadingText->SetVisible(false);

	globalSoundNode = new Node(context_);
	globalSound = globalSoundNode->CreateComponent<SoundSource>();

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
	audio->SetMasterGain("ALL", sSoundVolume);
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
		titleScreen->SetMenu(titleScreen->titleMenu);
		scene_->SetUpdateEnabled(false);
		game->ourUI->SetVisible(false);
		game->ourUI->SetEnabled(false);
		scene_->RemoveComponent(game);
		scene_->AddComponent(titleScreen, 777, LOCAL);
	}
	state = newState;
}

void GunPriest::Update(StringHash eventType, VariantMap& eventData)
{
	if (state == STATE_GAME)
	{
		if (input->GetKeyPress(KEY_ESCAPE))
		{
			ChangeState(STATE_TITLE);
		}
	}
	else if (state == STATE_TITLE)
	{
		if (input->GetKeyPress(KEY_RETURN))
		{
			ChangeState(STATE_GAME);
			if (!game->initialized)
			{
				StartGame();
			}
		}
		if (input->GetKeyPress(KEY_ESCAPE))
		{
			engine_->Exit();
			return;
		}
	}
}

void GunPriest::AfterRenderUpdate(StringHash eventType, VariantMap& eventData)
{
	if (state == STATE_GAME && game->initialized)
	{
		scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(debugRenderer, true);
	}
}

void GunPriest::Stop()
{
	
}
