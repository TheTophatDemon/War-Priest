#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>

using namespace Urho3D;

class Gameplay;
class TitleScreen : public LogicComponent
{
	URHO3D_OBJECT(TitleScreen, LogicComponent);
public:
	TitleScreen(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	void OnSetEnabled();
	~TitleScreen();
protected:

	Engine* engine_;
	SharedPtr<Renderer> renderer;
	SharedPtr<UI> ui;
	SharedPtr<Input> input;
	SharedPtr<ResourceCache> cache;
	SharedPtr<Audio> audio;
	SharedPtr<UIElement> ourUI;
};

