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
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/Button.h>
#include "Menu.h"

using namespace Urho3D;

class GunPriest;
class Gameplay;
class TitleScreen : public LogicComponent
{
	URHO3D_OBJECT(TitleScreen, LogicComponent);
public:
	TitleScreen(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	~TitleScreen();

	SharedPtr<UIElement> ourUI;
	SharedPtr<Gameplay> game;
	GunPriest* gunPriest;

	void MakeMenus();
	void SetMenu(Menu* newMenu);

	SharedPtr<Menu> titleMenu;
	SharedPtr<Menu> levelSelectMenu;
	SharedPtr<Menu> settingsMenu;

	SharedPtr<SoundSource> soundSource;
protected:
	void OnEvent(StringHash eventType, VariantMap& eventData);
	void OnUpdate(StringHash eventType, VariantMap& eventData);

	Engine* engine_;
	SharedPtr<Renderer> renderer;
	SharedPtr<UI> ui;
	SharedPtr<Input> input;
	SharedPtr<ResourceCache> cache;
	SharedPtr<Audio> audio;

	SharedPtr<Menu> currentMenu;
	SharedPtr<Node> soundNode;

	float time;
};

