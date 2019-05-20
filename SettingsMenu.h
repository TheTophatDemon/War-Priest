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
#include "GunPriest.h"
#include "Settings.h"

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

