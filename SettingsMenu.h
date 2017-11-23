#pragma once

#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Slider.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/ListView.h>

#include "Menu.h"
#include "GunPriest.h"

#define NUM_RESOLUTIONS 6

struct RebindButton
{
	SharedPtr<Button> button;
	int* setting = nullptr;
	int value;
};

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
	virtual void OnEnter();
	virtual void OnLeave();
	virtual void OnEvent(StringHash eventType, VariantMap& eventData);
	~SettingsMenu();
protected:
	void OnKeyPress(StringHash eventType, VariantMap& eventData);
	void OnMouseClick(StringHash eventType, VariantMap& eventData);
	void UpdateControls();
	void ApplySettings();
	void UpdateButtonLabel(RebindButton& butt);

	SharedPtr<Input> input;
	SharedPtr<Text> restartText;
	SharedPtr<UIElement> controlsPanel;

	SharedPtr<Slider> musicVolumeSlider;
	SharedPtr<Slider> soundVolumeSlider;
	SharedPtr<Slider> sensitivitySlider;

	SharedPtr<CheckBox> graphicsCheck;
	SharedPtr<CheckBox> bloodCheck;
	SharedPtr<CheckBox> invertMouseCheck;
	SharedPtr<CheckBox> vsyncCheck;
	SharedPtr<CheckBox> fullScreenCheck;

	SharedPtr<UIElement> resolutionList;
	ResolutionButton resButtons[NUM_RESOLUTIONS];
	int selectedRes;
	static Color selectedColor;
	static Color unSelectedColor;

	RebindButton rebindButtons[7];
	RebindButton* rebindButton;

	bool rebinding = false;
};

