#include "TitleScreen.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Button.h>
#include <iostream>

#include "Gameplay.h"

TitleScreen::TitleScreen(Context* context) : LogicComponent(context)
{
	cache = GetSubsystem<ResourceCache>();
	engine_ = GetSubsystem<Engine>();
	input = engine_->GetSubsystem<Input>();
	renderer = GetSubsystem<Renderer>();
	ui = GetSubsystem<UI>();
	audio = GetSubsystem<Audio>();
	ui->GetRoot()->LoadChildXML(cache->GetResource<XMLFile>("UI/titleLayout.xml")->GetRoot());
	ourUI = ui->GetRoot()->GetChild("titlescreen", true);
	
	//Disable all text so they don't interfere with button input
	PODVector<UIElement*> children;
	ourUI->GetChildren(children, true);
	for (PODVector<UIElement*>::Iterator i = children.Begin(); i != children.End(); ++i)
	{
		UIElement* element = (UIElement*)*i;
		if (element->GetName().Empty())
		{
			element->SetEnabled(false);
		}
	}

	input->SetMouseVisible(true);
	gotoGame = false;
}

void TitleScreen::RegisterObject(Context* context)
{
	context->RegisterFactory<TitleScreen>();
}

void TitleScreen::Start()
{
	gotoGame = false;
	input->SetMouseGrabbed(false);
	if (ourUI)
	{
		ourUI->SetEnabled(true);
		ourUI->SetVisible(true);
	}
	renderer->GetViewport(0)->SetRenderPath(cache->GetResource<XMLFile>("RenderPaths/Forward_Blur.xml"));
	input->SetMouseVisible(true);
	
	SubscribeToEvent(E_UIMOUSECLICKEND, URHO3D_HANDLER(TitleScreen, OnClick));
}

void TitleScreen::FixedUpdate(float timeStep)
{
	if (IsEnabled())
	{

	}
}

void TitleScreen::OnClick(StringHash eventType, VariantMap& eventData)
{
	UIElement* source = (UIElement*)eventData["Element"].GetPtr();
	if (source)
	{
		if (source->GetName() == "resumeGame")
		{
			gotoGame = true;
		}
		else if (source->GetName() == "startGame")
		{
			game->initialized = false;
			gotoGame = true;
		}
	}
}

TitleScreen::~TitleScreen()
{
}
