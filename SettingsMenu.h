#pragma once

#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Slider.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/ListView.h>

#include "Menu.h"
#include "GunPriest.h"
#include "Settings.h"

#define NUM_RESOLUTIONS 6

struct RebindButton
{
	SharedPtr<Button> button;
	SharedPtr<UInput>* setting = nullptr;
	SharedPtr<UInput> value;
	RebindButton(Button* butt, SharedPtr<UInput>* set) : button(butt), setting(set) {}
	RebindButton() {}
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
	virtual void OnEnter() override;
	virtual void OnLeave() override;
	virtual void OnEvent(StringHash eventType, VariantMap& eventData) override;
	~SettingsMenu();
protected:
	void OnKeyPress(StringHash eventType, VariantMap& eventData);
	void OnMouseClick(StringHash eventType, VariantMap& eventData);
	void UpdateControls();
	void ApplySettings();
	void UpdateButtonLabel(RebindButton& butt);

	SharedPtr<Input> input;
	SharedPtr<UIElement> controlsPanel;

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
	ResolutionButton resButtons[NUM_RESOLUTIONS];
	int selectedRes;
	static Color selectedColor;
	static Color unSelectedColor;

	RebindButton rebindButtons[Settings::numInputs];
	RebindButton* rebindButton;
	
	bool rebinding = false;
};

