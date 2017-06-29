#include "TitleScreen.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Core/CoreEvents.h>
#include <iostream>

#include "Gameplay.h"
#include "TitleMenu.h"
#include "LevelSelectMenu.h"
#include "SettingsMenu.h"
#include "GunPriest.h"
#include "Settings.h"

using namespace std;

TitleScreen::TitleScreen(Context* context) : LogicComponent(context)
{
	cache = GetSubsystem<ResourceCache>();
	engine_ = GetSubsystem<Engine>();
	input = engine_->GetSubsystem<Input>();
	renderer = GetSubsystem<Renderer>();
	ui = GetSubsystem<UI>();
	audio = GetSubsystem<Audio>();

	ourUI = ui->GetRoot()->CreateChild<UIElement>("titleUIParent");

	soundNode = new Node(context_);
	soundSource = soundNode->CreateComponent<SoundSource>();
	soundSource->SetSoundType("TITLE");

	input->SetMouseVisible(true);
}

void TitleScreen::MakeMenus()
{
	titleMenu = new TitleMenu(this, game);
	levelSelectMenu = new LevelSelectMenu(this, game);
	settingsMenu = new SettingsMenu(this, game);
	SetMenu(titleMenu);
}

void TitleScreen::RegisterObject(Context* context)
{
	context->RegisterFactory<TitleScreen>();
}

void TitleScreen::Start()
{
	audio->SetMasterGain("GAMEPLAY", 0.0f);
	audio->SetMasterGain("TITLE", Settings::GetSoundVolume());

	if (ourUI)
	{
		ourUI->SetEnabled(true);
		ourUI->SetVisible(true);
	}
	
	renderer->GetViewport(0)->SetRenderPath(cache->GetResource<XMLFile>("RenderPaths/Forward_Blur.xml"));
	input->SetMouseVisible(true);
	input->SetMouseGrabbed(false);
	
	SubscribeToEvent(E_UIMOUSECLICKEND, URHO3D_HANDLER(TitleScreen, OnEvent));
	SubscribeToEvent(E_UIMOUSECLICK, URHO3D_HANDLER(TitleScreen, OnEvent));
	SubscribeToEvent(E_HOVERBEGIN, URHO3D_HANDLER(TitleScreen, OnEvent));
	SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(TitleScreen, OnEvent));
	SubscribeToEvent(E_SLIDERCHANGED, URHO3D_HANDLER(TitleScreen, OnEvent));
	SubscribeToEvent(E_TOGGLED, URHO3D_HANDLER(TitleScreen, OnEvent));

	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(TitleScreen, OnUpdate));
}

void TitleScreen::OnUpdate(StringHash eventType, VariantMap& eventData)
{
	float timeStep = eventData["TimeStep"].GetFloat();
	if (IsEnabled())
	{
		audio->SetMasterGain("TITLE", Settings::GetSoundVolume());
		currentMenu->Update(timeStep);
	}
}

void TitleScreen::OnEvent(StringHash eventType, VariantMap& eventData)
{
	currentMenu->OnEvent(eventType, eventData);
	//Play those sounds, dude.
	Button* button = dynamic_cast<Button*>(eventData["Element"].GetPtr());
	if (button) 
	{
		if (eventType == E_UIMOUSECLICK)
		{
			soundSource->Play(cache->GetResource<Sound>("Sounds/gui_select.wav"));
		}
		else if (eventType == E_HOVERBEGIN)
		{
			soundSource->Play(cache->GetResource<Sound>("Sounds/gui_hover.wav"));
		}
	}
}

void TitleScreen::SetMenu(Menu* newMenu)
{
	if (currentMenu) currentMenu->OnLeave();
	currentMenu = newMenu;
	currentMenu->OnEnter();
}

TitleScreen::~TitleScreen()
{
}
