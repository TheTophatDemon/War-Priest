/*
Copyright (C) 2021 Alexander Lunsford

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Slider.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/ListView.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/ScrollView.h>
#include <Urho3D/UI/ScrollBar.h>

#include "Menu.h"
#include "../GunPriest.h"
#include "../Settings.h"

enum class BindingMode
{
	KEYB_MOUSE,
	JOYSTICK
};

class RebindScreen : public GP::Menu
{
	friend class SettingsMenu;

	URHO3D_OBJECT(RebindScreen, GP::Menu);
public:
	RebindScreen(Context* context, TitleScreen* titleScreen, SharedPtr<Gameplay> gm);
	virtual void OnEnter() override;
	virtual void Update(float timeStep) override;
	~RebindScreen();
protected:
	void OnEvent(StringHash eventType, VariantMap& eventData);
	void SyncControls();
	void SetRebinding(const bool mode);
	
	SharedPtr<Gameplay> gameplay;
	SharedPtr<Input> input;
	SharedPtr<UI> ui;
	SharedPtr<ResourceCache> cache;
	
	SharedPtr<Window> window;
	SharedPtr<ListView> panel;
	SharedPtr<Text> caption;
	Vector<SharedPtr<Button>> rebindButtons;
	Button* rebindButton;
	JoystickState prevJoyState;
	int lastJoyIndex;
	BindingMode bindingMode;
	//Timeout timer
	float rebindTimer = 0.0f;
	bool rebinding = false;
};

