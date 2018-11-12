#include "SettingsMenu.h"
#include "TitleScreen.h"

#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Audio/Audio.h>
#include <iostream>

Color SettingsMenu::selectedColor = Color(0.65f, 0.75f, 0.65f);
Color SettingsMenu::unSelectedColor = Color(0.25f, 0.25f, 0.25f);

SettingsMenu::SettingsMenu(TitleScreen* ts, SharedPtr<Gameplay> gm) : GP::Menu(ts, gm), selectedRes(0)
{
	layoutPath = "UI/titlemenus/settingsScreen.xml";
}

void SettingsMenu::OnEnter()
{
	Settings::LoadSettings(titleScreen->GetContext());
	GP::Menu::OnEnter();
	input = titleScreen->GetSubsystem<Input>();

	controlsPanel = titleScreen->ourUI->GetChild("controlsPanel", true);
	musicVolumeSlider = titleScreen->ourUI->GetChildDynamicCast<Slider>("musicVolumeSlider", true);
	soundVolumeSlider = titleScreen->ourUI->GetChildDynamicCast<Slider>("soundVolumeSlider", true);
	sensitivitySlider = titleScreen->ourUI->GetChildDynamicCast<Slider>("sensitivitySlider", true);
	difficultySlider = titleScreen->ourUI->GetChildDynamicCast<Slider>("difficultySlider", true);

	graphicsCheck = titleScreen->ourUI->GetChildDynamicCast<CheckBox>("graphicsCheck", true);
	bloodCheck = titleScreen->ourUI->GetChildDynamicCast<CheckBox>("bloodCheck", true);
	invertMouseCheck = titleScreen->ourUI->GetChildDynamicCast<CheckBox>("invertMouseCheck", true);
	vsyncCheck = titleScreen->ourUI->GetChildDynamicCast<CheckBox>("vsyncCheck", true);
	fullScreenCheck = titleScreen->ourUI->GetChildDynamicCast<CheckBox>("fullScreenCheck", true);

	//Add buttons for each resolution
	resolutionList = titleScreen->ourUI->GetChild("resolutionList", true);
	resolutionList->SetClipChildren(true);
	resolutionList->SetClipBorder(IntRect(4, 4, 4, 4));
	String resLabels[] = {"1920x1080", "1280x720", "800x600", "800x450", "640x480", "640x360"};
	const int pResX[] = { 1920, 1280, 800, 800, 640, 640 };
	const int pResY[] = { 1080, 720, 600, 450, 480, 360 };
	const int buttHeight = resolutionList->GetSize().y_ / NUM_RESOLUTIONS;
	for (int i = 0; i < NUM_RESOLUTIONS; ++i)
	{
		Button* butt = (Button*)resolutionList->CreateChild(Button::GetTypeStatic());
		butt->SetSize(resolutionList->GetSize().x_, buttHeight);
		butt->SetPosition(0, buttHeight*i);
		butt->SetColor(unSelectedColor);
		Text* text = (Text*)butt->CreateChild(Text::GetTypeStatic());
		text->SetText(resLabels[i]);
		text->SetFont("Fonts/Anonymous Pro.ttf", 20);
		text->SetHorizontalAlignment(HA_CENTER);
		text->SetVerticalAlignment(VA_CENTER);
		text->SetColor(Color::WHITE);
		resButtons[i].button = butt;
		resButtons[i].label = resLabels[i];
		resButtons[i].resX = pResX[i];
		resButtons[i].resY = pResY[i];
	}

	UpdateControls();
}

void SettingsMenu::UpdateControls() //Syncs the ui controls to the actual settings
{
	musicVolumeSlider->SetValue(Settings::GetMusicVolume() * musicVolumeSlider->GetRange());
	soundVolumeSlider->SetValue(Settings::GetSoundVolume() * soundVolumeSlider->GetRange());
	sensitivitySlider->SetValue(Settings::GetMouseSensitivity() * sensitivitySlider->GetRange());
	difficultySlider->SetValue((Settings::GetDifficulty() - 0.5f) * difficultySlider->GetRange());
	
	graphicsCheck->SetChecked(Settings::AreGraphicsFast());
	bloodCheck->SetChecked(Settings::IsBloodEnabled());
	invertMouseCheck->SetChecked(Settings::IsMouseInverted());
	vsyncCheck->SetChecked(Settings::IsVsync());
	fullScreenCheck->SetChecked(Settings::IsFullScreen());
	
	for (int i = 0; i < NUM_RESOLUTIONS; ++i)
	{
		if (resButtons[i].resX == Settings::GetResolutionX() && resButtons[i].resY == Settings::GetResolutionY())
		{
			resButtons[i].button->SetColor(selectedColor);
			selectedRes = i;
		}
		else
		{
			resButtons[i].button->SetColor(unSelectedColor);
		}
	}
}

void SettingsMenu::OnEvent(StringHash eventType, VariantMap& eventData)
{
	if (eventType == E_UIMOUSECLICKEND) 
	{
		OnMouseClick(eventType, eventData);
	}
}

void SettingsMenu::OnMouseClick(StringHash eventType, VariantMap& eventData)
{
	int mouseButtonPressed = eventData["Button"].GetInt();
	Button* source = dynamic_cast<Button*>(eventData["Element"].GetPtr());
	if (source)
	{
		if (source->GetParent() == resolutionList)
		{
			for (int i = 0; i < NUM_RESOLUTIONS; ++i)
			{
				if (resButtons[i].button == source) 
				{
					resButtons[i].button->SetColor(selectedColor);
					selectedRes = i;
				}
				else 
				{
					resButtons[i].button->SetColor(unSelectedColor);
				}
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
				Settings::RevertSettings(titleScreen->GetContext());
				UpdateControls();
				Settings::SaveSettings(titleScreen->GetContext());
				titleScreen->gunPriest->VideoSetup();
				ui->SetWidth(1280);
				titleScreen->SetMenu(titleScreen->titleMenu);
			}
		}
	}
}

void SettingsMenu::ApplySettings()
{
	Settings::musicVolume = musicVolumeSlider->GetValue() / musicVolumeSlider->GetRange();
	Settings::soundVolume = soundVolumeSlider->GetValue() / soundVolumeSlider->GetRange();
	Settings::mouseSensitivity = sensitivitySlider->GetValue() / sensitivitySlider->GetRange();
	Settings::difficulty = (difficultySlider->GetValue() / difficultySlider->GetRange()) + 0.5f;

	Settings::bloodEnabled = bloodCheck->IsChecked();
	Settings::mouseInvert = invertMouseCheck->IsChecked();
	Settings::fastGraphics = graphicsCheck->IsChecked();
	Settings::vSync = vsyncCheck->IsChecked();
	Settings::fullScreen = fullScreenCheck->IsChecked();

	Settings::xRes = resButtons[selectedRes].resX;
	Settings::yRes = resButtons[selectedRes].resY;

	titleScreen->gunPriest->VideoSetup();
	ui->SetWidth(1280);
	
	titleScreen->SendEvent(Settings::E_SETTINGSCHANGED, VariantMap());
}
void SettingsMenu::OnLeave()
{
	Audio* audio = titleScreen->GetSubsystem<Audio>();
	audio->SetMasterGain("TITLE", Settings::GetSoundVolume());
	audio->SetMasterGain("MUSIC", Settings::GetMusicVolume());
}

SettingsMenu::~SettingsMenu()
{
}
