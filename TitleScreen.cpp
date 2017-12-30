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
#include "GunPriest.h"
#include "Settings.h"
#include "TitleMenu.h"
#include "LevelSelectMenu.h"
#include "SettingsMenu.h"

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

	time = 0.0f;

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
	audio->SetMasterGain("MUSIC", Settings::GetMusicVolume());

	ui->SetWidth(1280);
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
	time += timeStep;
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
	if (eventType == E_TOGGLED && time > 0.1f)
	{
		soundSource->Play(cache->GetResource<Sound>("Sounds/gui_check.wav"));
	}
	else if (eventType == E_SLIDERCHANGED && time > 0.1f)
	{
		soundSource->Play(cache->GetResource<Sound>("Sounds/gui_scroll.wav"));
	}
	else 
	{
		Button* button = dynamic_cast<Button*>(eventData["Element"].GetPtr());
		if (button)
		{
			if (eventType == E_UIMOUSECLICK)
			{
				soundSource->Play(cache->GetResource<Sound>("Sounds/gui_select.wav"));
			}
			else if (eventType == E_HOVERBEGIN)
			{
				soundSource->Play(cache->GetResource<Sound>("Sounds/gui_hover.wav"), 44100.0f + Random(-1000.0f, 1000.0f));
			}
		}
	}
}

void TitleScreen::SetMenu(GP::Menu* newMenu)
{
	time = 0.0f;
	if (currentMenu) currentMenu->OnLeave();
	currentMenu = newMenu;
	currentMenu->OnEnter();
}

TitleScreen::~TitleScreen()
{
}
