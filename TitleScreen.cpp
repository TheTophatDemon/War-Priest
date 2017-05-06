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
#include "GunPriest.h"

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

	input->SetMouseVisible(true);
}

void TitleScreen::MakeMenus()
{
	titleMenu = new TitleMenu(this, game);
	levelSelectMenu = new LevelSelectMenu(this, game);
	SetMenu(titleMenu);
}

void TitleScreen::RegisterObject(Context* context)
{
	context->RegisterFactory<TitleScreen>();
}

void TitleScreen::Start()
{
	if (ourUI)
	{
		ourUI->SetEnabled(true);
		ourUI->SetVisible(true);
	}
	
	renderer->GetViewport(0)->SetRenderPath(cache->GetResource<XMLFile>("RenderPaths/Forward_Blur.xml"));
	input->SetMouseVisible(true);
	input->SetMouseGrabbed(false);
	
	SubscribeToEvent(E_UIMOUSECLICKEND, URHO3D_HANDLER(TitleScreen, OnClick));
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(TitleScreen, OnUpdate));
}

void TitleScreen::OnUpdate(StringHash eventType, VariantMap& eventData)
{
	float timeStep = eventData["TimeStep"].GetFloat();
	if (IsEnabled())
	{
		currentMenu->Update(timeStep);
	}
}

void TitleScreen::OnClick(StringHash eventType, VariantMap& eventData)
{
	currentMenu->OnClick(eventType, eventData);
}

void TitleScreen::SetMenu(Menu* newMenu)
{
	currentMenu = newMenu;
	currentMenu->OnEnter();
}

TitleScreen::~TitleScreen()
{
}
