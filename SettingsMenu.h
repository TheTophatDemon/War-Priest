#pragma once

#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Slider.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/CheckBox.h>
#include "Menu.h"
#include "GunPriest.h"

class SettingsMenu : public Menu
{
public:
	SettingsMenu(TitleScreen* ts, SharedPtr<Gameplay> gm);
	virtual void OnEnter();
	virtual void OnEvent(StringHash eventType, VariantMap& eventData);
	~SettingsMenu();
protected:
	void OnKeyPress(StringHash eventType, VariantMap& eventData);
	void OnMouseClick(StringHash eventType, VariantMap& eventData);
	void UpdateControls();
	void ApplySettings();
	void ChangeBinding(Button* button, int newButton);
	void UpdateButtonLabel(Button* button, int butt);

	SharedPtr<Input> input;
	SharedPtr<Text> restartText;
	SharedPtr<UIElement> controlsPanel;
	SharedPtr<Button> rebindButton;

	SharedPtr<Slider> musicVolumeSlider;
	SharedPtr<Slider> soundVolumeSlider;
	SharedPtr<Slider> sensitivitySlider;

	SharedPtr<CheckBox> graphicsCheck;
	SharedPtr<CheckBox> bloodCheck;
	SharedPtr<CheckBox> invertMouseCheck;
	SharedPtr<CheckBox> vsyncCheck;
	SharedPtr<CheckBox> fullScreenCheck;

	bool rebinding = false;
};

