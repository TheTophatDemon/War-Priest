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
public:
	SettingsMenu(TitleScreen* ts, SharedPtr<Gameplay> gm);
	virtual void OnEnter() override;
	virtual void OnLeave() override;
	virtual void Update(float timeStep) override;
	virtual void OnEvent(StringHash eventType, VariantMap& eventData) override;
	~SettingsMenu();
protected:
	void SyncControls();

	SharedPtr<Audio> audio;
	SharedPtr<Input> input;

	SharedPtr<Button> controlsButton;

	SharedPtr<Window> rebindWindow;
	SharedPtr<ListView> rebindPanel;
	SharedPtr<Text> rebindLabel;
	//SharedPtr<ScrollBar> rebindScroll;
	Vector<SharedPtr<Button>> rebindButtons;

	SharedPtr<Slider> musicVolumeSlider;
	SharedPtr<Slider> soundVolumeSlider;
	SharedPtr<Slider> sensitivitySlider;
	SharedPtr<Slider> difficultySlider;

	SharedPtr<CheckBox> graphicsCheck;
	SharedPtr<CheckBox> bloodCheck;
	SharedPtr<CheckBox> invertMouseCheck;
	SharedPtr<CheckBox> vsyncCheck;
	SharedPtr<CheckBox> fullScreenCheck;

	SharedPtr<UIElement> resolutionList;
	ResolutionButton resButtons[Settings::NUM_RESOLUTIONS];
	int selectedRes;

	const static Color selectedColor;
	const static Color unSelectedColor;

	bool videoSettingDirty = false;

	bool rebinding = false;
	Button* rebindButton;
};

