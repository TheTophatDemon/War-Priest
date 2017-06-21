#pragma once

#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Slider.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/Text.h>
#include "Menu.h"
#include "GunPriest.h"

class SettingsMenu : public Menu
{
public:
	SettingsMenu(TitleScreen* ts, SharedPtr<Gameplay> gm);
	virtual void OnEnter();
	virtual void Update(float timeStep);
	virtual void OnClick(StringHash eventType, VariantMap& eventData);
	~SettingsMenu();
};

