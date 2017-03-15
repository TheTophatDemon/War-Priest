#include "TitleScreen.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <iostream>

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
	input->SetMouseVisible(true);
}

void TitleScreen::RegisterObject(Context* context)
{
	context->RegisterFactory<TitleScreen>();
}

void TitleScreen::Start()
{
	input->SetMouseGrabbed(false);
	if (ourUI)
	{
		ourUI->SetEnabledRecursive(true);
		ourUI->SetVisible(true);
	}
	renderer->GetViewport(0)->SetRenderPath(cache->GetResource<XMLFile>("RenderPaths/Forward_Blur.xml"));
	input->SetMouseVisible(true);
}

void TitleScreen::FixedUpdate(float timeStep)
{
	if (IsEnabled())
	{

	}
}

TitleScreen::~TitleScreen()
{
}
