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

struct ResolutionButton
{
	WeakPtr<Button> button;
	String label;
	int resX;
	int resY;
};

class SettingsMenu : public GP::Menu
{
	URHO3D_OBJECT(SettingsMenu, GP::Menu);
public:
	SettingsMenu(Context* context, TitleScreen* ts, SharedPtr<Gameplay> gm);
	virtual void OnEnter() override;
	virtual void OnLeave() override;
	virtual void Update(float timeStep) override;
	~SettingsMenu();
protected:
	void GenerateUI();
	void OnEvent(StringHash eventType, VariantMap& eventData);
	void SyncControls();

	SharedPtr<Audio> audio;
	SharedPtr<Input> input;

	SharedPtr<RebindScreen> rebindScreen;
	SharedPtr<Button> controlsButton;

	SharedPtr<Slider> musicVolumeSlider;
	SharedPtr<Slider> soundVolumeSlider;
	SharedPtr<Slider> sensitivitySlider;
	SharedPtr<Slider> difficultySlider;

	SharedPtr<CheckBox> graphicsCheck;
	SharedPtr<CheckBox> bloodCheck;
	SharedPtr<CheckBox> invertMouseCheck;
	SharedPtr<CheckBox> vsyncCheck;
	SharedPtr<CheckBox> fullScreenCheck;

	SharedPtr<Text> difficultyWarning;

	SharedPtr<UIElement> resolutionList;
	ResolutionButton resButtons[Settings::NUM_RESOLUTIONS];
	int selectedRes;

	const static Color selectedColor;
	const static Color unSelectedColor;

	bool videoSettingDirty = false;
};

