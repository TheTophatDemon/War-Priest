#include "SettingsMenu.h"
#include "TitleScreen.h"
#include "Settings.h"

#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <iostream>

SettingsMenu::SettingsMenu(TitleScreen* ts, SharedPtr<Gameplay> gm) : Menu(ts, gm)
{
	layoutPath = "UI/titlemenus/settingsScreen.xml";
}

void SettingsMenu::OnEnter()
{
	Settings::LoadSettings(titleScreen->GetContext());
	Menu::OnEnter();
	input = titleScreen->GetSubsystem<Input>();

	restartText = titleScreen->ourUI->GetChildDynamicCast<Text>("restartText", true);
	restartText->SetVisible(false);

	controlsPanel = titleScreen->ourUI->GetChild("controlsPanel", true);
	musicVolumeSlider = titleScreen->ourUI->GetChildDynamicCast<Slider>("musicVolumeSlider", true);
	soundVolumeSlider = titleScreen->ourUI->GetChildDynamicCast<Slider>("soundVolumeSlider", true);
	sensitivitySlider = titleScreen->ourUI->GetChildDynamicCast<Slider>("sensitivitySlider", true);

	graphicsCheck = titleScreen->ourUI->GetChildDynamicCast<CheckBox>("graphicsCheck", true);
	bloodCheck = titleScreen->ourUI->GetChildDynamicCast<CheckBox>("bloodCheck", true);
	invertMouseCheck = titleScreen->ourUI->GetChildDynamicCast<CheckBox>("invertMouseCheck", true);
	vsyncCheck = titleScreen->ourUI->GetChildDynamicCast<CheckBox>("vsyncCheck", true);
	fullScreenCheck = titleScreen->ourUI->GetChildDynamicCast<CheckBox>("fullScreenCheck", true);

	UpdateControls();
}

void SettingsMenu::UpdateControls()
{
	musicVolumeSlider->SetValue(Settings::GetMusicVolume() * musicVolumeSlider->GetRange());
	soundVolumeSlider->SetValue(Settings::GetSoundVolume() * soundVolumeSlider->GetRange());
	sensitivitySlider->SetValue(Settings::GetMouseSensitivity() * sensitivitySlider->GetRange());
	
	graphicsCheck->SetChecked(Settings::AreGraphicsFast());
	bloodCheck->SetChecked(Settings::IsBloodEnabled());
	invertMouseCheck->SetChecked(Settings::IsMouseInverted());
	vsyncCheck->SetChecked(Settings::IsVsync());
	fullScreenCheck->SetChecked(Settings::IsFullScreen());
	
	UpdateButtonLabel(controlsPanel->GetChildDynamicCast<Button>("forwardButton", false), Settings::GetForwardKey());
	UpdateButtonLabel(controlsPanel->GetChildDynamicCast<Button>("backwardButton", false), Settings::GetBackwardKey());
	UpdateButtonLabel(controlsPanel->GetChildDynamicCast<Button>("leftButton", false), Settings::GetLeftKey());
	UpdateButtonLabel(controlsPanel->GetChildDynamicCast<Button>("rightButton", false), Settings::GetRightKey());
	UpdateButtonLabel(controlsPanel->GetChildDynamicCast<Button>("slideButton", false), Settings::GetSlideKey());
	UpdateButtonLabel(controlsPanel->GetChildDynamicCast<Button>("reviveButton", false), Settings::GetReviveKey());
	UpdateButtonLabel(controlsPanel->GetChildDynamicCast<Button>("jumpButton", false), Settings::GetJumpKey());
}

void SettingsMenu::OnEvent(StringHash eventType, VariantMap& eventData)
{
	if (eventType == E_UIMOUSECLICKEND) 
	{
		OnMouseClick(eventType, eventData);
	}
	else if (eventType == E_KEYDOWN)
	{
		OnKeyPress(eventType, eventData);
	}
}

void SettingsMenu::OnMouseClick(StringHash eventType, VariantMap& eventData)
{
	int mouseButtonPressed = eventData["Button"].GetInt();

	if (rebinding)
	{
		if (mouseButtonPressed == MOUSEB_LEFT)
		{
			ChangeBinding(rebindButton, KEY_SCROLLLOCK);
		}
		else if (mouseButtonPressed == MOUSEB_RIGHT)
		{
			ChangeBinding(rebindButton, KEY_RGUI);
		}
		else if (mouseButtonPressed == MOUSEB_MIDDLE)
		{
			ChangeBinding(rebindButton, KEY_PAUSE);
		}
		rebinding = false;
	}
	else
	{
		Button* source = dynamic_cast<Button*>(eventData["Element"].GetPtr());
		if (source)
		{
			if (source->GetParent() == controlsPanel)
			{
				rebinding = true;
				rebindButton = source;
			}
			else
			{
				if (source->GetName() == "cancelButton")
				{
					titleScreen->SetMenu(titleScreen->titleMenu);
				}
				else if (source->GetName() == "confirmButton")
				{
					ApplySettings();
					Settings::SaveSettings(titleScreen->GetContext());
					titleScreen->SetMenu(titleScreen->titleMenu);
				}
				else if (source->GetName() == "revertButton")
				{
					Settings::RevertSettings();
					UpdateControls(); //So it'll update the sliders n' whatnot.
				}
			}
		}
	}
}

void SettingsMenu::OnKeyPress(StringHash eventType, VariantMap& eventData)
{
	int keyPressed = eventData["Key"].GetInt();
	if (rebinding)
	{
		ChangeBinding(rebindButton, keyPressed);
		rebinding = false;
	}
}

void SettingsMenu::ChangeBinding(Button* button, int newButton)
{
	if (button->GetName() == "forwardButton")
		Settings::keyForward = newButton;
	else if (button->GetName() == "backwardButton")
		Settings::keyBackward = newButton;
	else if (button->GetName() == "rightButton")
		Settings::keyRight = newButton;
	else if (button->GetName() == "leftButton")
		Settings::keyLeft = newButton;
	else if (button->GetName() == "jumpButton")
		Settings::keyJump = newButton;
	else if (button->GetName() == "reviveButton")
		Settings::keyRevive = newButton;
	else if (button->GetName() == "slideButton")
		Settings::keySlide = newButton;
	UpdateButtonLabel(button, newButton);
	rebinding = false;
}

void SettingsMenu::UpdateButtonLabel(Button* button, int butt)
{
	Text* label = button->GetChildDynamicCast<Text>("label", false);
	assert(label);
	switch (butt)
	{
	case KEY_SCROLLLOCK:
		label->SetText("LMB");
		break;
	case KEY_RGUI:
		label->SetText("RMB");
		break;
	case KEY_PAUSE:
		label->SetText("MMB");
		break;
	default:
		label->SetText(input->GetKeyName(butt));
		break;
	}
}

void SettingsMenu::ApplySettings()
{
	Settings::musicVolume = musicVolumeSlider->GetValue() / musicVolumeSlider->GetRange();
	Settings::soundVolume = soundVolumeSlider->GetValue() / soundVolumeSlider->GetRange();
	Settings::mouseSensitivity = sensitivitySlider->GetValue() / sensitivitySlider->GetRange();

	Settings::fastGraphics = graphicsCheck->IsChecked();
	Settings::bloodEnabled = bloodCheck->IsChecked();
	Settings::mouseInvert = invertMouseCheck->IsChecked();
	Settings::vSync = vsyncCheck->IsChecked();
	Settings::fullScreen = fullScreenCheck->IsChecked();
}

SettingsMenu::~SettingsMenu()
{
}
