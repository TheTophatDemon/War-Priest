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

	rebindButtons[0].button = controlsPanel->GetChildDynamicCast<Button>("forwardButton", false);
		rebindButtons[0].setting = &Settings::keyForward;
	rebindButtons[1].button = controlsPanel->GetChildDynamicCast<Button>("backwardButton", false);
		rebindButtons[1].setting = &Settings::keyBackward;
	rebindButtons[2].button = controlsPanel->GetChildDynamicCast<Button>("leftButton", false);
		rebindButtons[2].setting = &Settings::keyLeft;
	rebindButtons[3].button = controlsPanel->GetChildDynamicCast<Button>("rightButton", false);
		rebindButtons[3].setting = &Settings::keyRight;
	rebindButtons[4].button = controlsPanel->GetChildDynamicCast<Button>("slideButton", false);
		rebindButtons[4].setting = &Settings::keySlide;
	rebindButtons[5].button = controlsPanel->GetChildDynamicCast<Button>("reviveButton", false);
		rebindButtons[5].setting = &Settings::keyRevive;
	rebindButtons[6].button = controlsPanel->GetChildDynamicCast<Button>("jumpButton", false);
		rebindButtons[6].setting = &Settings::keyJump;

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
	
	for (int i = 0; i < 7; ++i)
	{
		rebindButtons[i].value = *rebindButtons[i].setting;
		UpdateButtonLabel(rebindButtons[i]);
	}
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
			rebindButton->value = KEY_SCROLLLOCK;
		}
		else if (mouseButtonPressed == MOUSEB_RIGHT)
		{
			rebindButton->value = KEY_RGUI;
		}
		else if (mouseButtonPressed == MOUSEB_MIDDLE)
		{
			rebindButton->value = KEY_PAUSE;
		}
		UpdateButtonLabel(*rebindButton);
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
				for (int i = 0; i < 7; ++i)
				{
					if (rebindButtons[i].button == source)
						rebindButton = &rebindButtons[i];
				}
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
					Settings::SaveSettings(titleScreen->GetContext());
					titleScreen->SetMenu(titleScreen->titleMenu);
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
		rebindButton->value = keyPressed;
		UpdateButtonLabel(*rebindButton);
		rebinding = false;
	}
}

void SettingsMenu::UpdateButtonLabel(RebindButton& butt)
{
	Text* label = butt.button->GetChildDynamicCast<Text>("label", false);
	assert(label);
	switch (butt.value)
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
		label->SetText(input->GetKeyName(butt.value));
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

	for (int i = 0; i < 7; ++i)
	{
		*rebindButtons[i].setting = rebindButtons[i].value;
	}
}

SettingsMenu::~SettingsMenu()
{
}
