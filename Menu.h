#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/Button.h>
#include "Gameplay.h"

using namespace Urho3D;

class TitleScreen;
class Menu : public RefCounted
{
public:
	Menu(TitleScreen* ts, SharedPtr<Gameplay> gm);
	virtual void OnEnter() = 0;
	virtual void Update(float timeStep) = 0;
	virtual void OnClick(StringHash eventType, VariantMap& eventData) = 0;
	~Menu();
protected:
	SharedPtr<TitleScreen> titleScreen;
	SharedPtr<Gameplay> gameplay;
	SharedPtr<ResourceCache> cache;
	void DisableTexts();
};

