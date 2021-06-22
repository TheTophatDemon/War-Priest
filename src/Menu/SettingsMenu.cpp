#include "SettingsMenu.h"
#include "TitleScreen.h"

#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Audio/Audio.h>
#include <iostream>

#include "RebindScreen.h"

const Color SettingsMenu::selectedColor = Color(0.65f, 0.75f, 0.65f);
const Color SettingsMenu::unSelectedColor = Color(0.25f, 0.25f, 0.25f);

SettingsMenu::SettingsMenu(Context* context, TitleScreen* ts, SharedPtr<Gameplay> gm) : GP::Menu(context, ts, gm), 
	selectedRes(0), videoSettingDirty(false)
{
	layoutPath = "UI/titlemenus/settingsScreen.xml";

	input = titleScreen->GetSubsystem<Input>();
	audio = titleScreen->GetSubsystem<Audio>();

	SubscribeToEvent(E_UIMOUSECLICKEND, URHO3D_HANDLER(SettingsMenu, OnEvent));
	SubscribeToEvent(E_TOGGLED, URHO3D_HANDLER(SettingsMenu, OnEvent));
}

void SettingsMenu::OnEnter()
{
	Settings::LoadSettings(titleScreen->GetContext());
	videoSettingDirty = false;
	GenerateUI();
}

void SettingsMenu::GenerateUI()
{
	GP::Menu::OnEnter();

	controlsButton = titleScreen->ourUI->GetChildDynamicCast<Button>("controlsButton", true);

	musicVolumeSlider = titleScreen->ourUI->GetChildDynamicCast<Slider>("musicVolumeSlider", true);
	soundVolumeSlider = titleScreen->ourUI->GetChildDynamicCast<Slider>("soundVolumeSlider", true);
	sensitivitySlider = titleScreen->ourUI->GetChildDynamicCast<Slider>("sensitivitySlider", true);
	difficultySlider = titleScreen->ourUI->GetChildDynamicCast<Slider>("difficultySlider", true);

	graphicsCheck = titleScreen->ourUI->GetChildDynamicCast<CheckBox>("graphicsCheck", true);
	bloodCheck = titleScreen->ourUI->GetChildDynamicCast<CheckBox>("bloodCheck", true);
	invertMouseCheck = titleScreen->ourUI->GetChildDynamicCast<CheckBox>("invertMouseCheck", true);
	vsyncCheck = titleScreen->ourUI->GetChildDynamicCast<CheckBox>("vsyncCheck", true);
	fullScreenCheck = titleScreen->ourUI->GetChildDynamicCast<CheckBox>("fullScreenCheck", true);

	difficultyWarning = titleScreen->ourUI->GetChildDynamicCast<Text>("warnText", true);
	difficultyWarning->SetVisible(false);

	//Add buttons for each resolution
	resolutionList = titleScreen->ourUI->GetChild("resolutionList", true);
	resolutionList->SetClipChildren(true);
	//resolutionList->SetClipBorder(IntRect(6, 6, 6, 6));
	const int buttHeight = (resolutionList->GetSize().y_ - 12) / Settings::NUM_RESOLUTIONS;
	for (int i = 0; i < Settings::NUM_RESOLUTIONS; ++i)
	{
		Button* butt = (Button*)resolutionList->CreateChild(Button::GetTypeStatic());
		butt->SetStyleAuto(ui->GetRoot()->GetDefaultStyle());
		butt->SetSize(resolutionList->GetSize().x_ - 12, buttHeight);
		butt->SetHorizontalAlignment(HA_CENTER);
		butt->SetPosition(0, buttHeight*i + 6);
		butt->SetColor(unSelectedColor);
		String label = String(Settings::RES_X[i]) + "x" + String(Settings::RES_Y[i]);
		Text* text = (Text*)butt->CreateChild(Text::GetTypeStatic());
		text->SetText(label);
		text->SetFont("Fonts/Anonymous Pro.ttf", 20);
		text->SetHorizontalAlignment(HA_CENTER);
		text->SetVerticalAlignment(VA_CENTER);
		text->SetColor(Color::WHITE);
		resButtons[i].button = butt;
		resButtons[i].label = label;
		resButtons[i].resX = Settings::RES_X[i];
		resButtons[i].resY = Settings::RES_Y[i];
	}

	SyncControls();
}

//Syncs the ui controls to the actual settings
void SettingsMenu::SyncControls()
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

	for (int i = 0; i < Settings::NUM_RESOLUTIONS; ++i)
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

//Syncs current settings to UI elements
void SettingsMenu::Update(float timeStep)
{
	if (rebindScreen.NotNull()) rebindScreen->Update(timeStep);

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

	difficultyWarning->SetVisible(Settings::difficulty >= Settings::UNHOLY_THRESHOLD);

	//Update volumes
	audio->SetMasterGain("GAMEPLAY", Settings::GetSoundVolume());
	audio->SetMasterGain("MUSIC", Settings::GetMusicVolume());
}

void SettingsMenu::OnEvent(StringHash eventType, VariantMap& eventData)
{
	if (eventType == E_UIMOUSECLICKEND)
	{
		int mouseButtonPressed = eventData["Button"].GetInt();
		Button* source = dynamic_cast<Button*>(eventData["Element"].GetPtr());
		if (!source) return;
		if (source->GetParent() == resolutionList)
		{
			for (int i = 0; i < Settings::NUM_RESOLUTIONS; ++i)
			{
				if (resButtons[i].button == source)
				{
					resButtons[i].button->SetColor(selectedColor);
					if (i != selectedRes) videoSettingDirty = true;
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
				Settings::LoadSettings(titleScreen->GetContext());
				titleScreen->SetMenu(titleScreen->titleMenu);
			}
			else if (source->GetName() == "confirmButton")
			{
				if (videoSettingDirty) titleScreen->gunPriest->VideoSetup();
				Settings::SaveSettings(titleScreen->GetContext());
				titleScreen->SetMenu(titleScreen->titleMenu);
			}
			else if (source->GetName() == "revertButton")
			{
				int oldX = Settings::GetResolutionX();
				int oldY = Settings::GetResolutionY();
				Settings::RevertSettings(titleScreen->GetContext());
				if (Settings::GetResolutionX() != oldX || Settings::GetResolutionY() != oldY) videoSettingDirty = true;
				SyncControls();
			}
			else if (source->GetName() == "controlsButton")
			{
				rebindScreen = new RebindScreen(context_, titleScreen, gameplay);
				rebindScreen->OnEnter();
			}
			else if (rebindScreen.NotNull() && source->GetName() == "closeButton")
			{
				if (source->GetParent()->GetName() == "rebindScreen")
				{
					rebindScreen->OnLeave();
					rebindScreen = nullptr;
					GenerateUI();
				}
			}
		}
	}
	else if (eventType == E_TOGGLED)
	{
		CheckBox* source = dynamic_cast<CheckBox*>(eventData["Element"].GetPtr());
		if (!source) return;
		if (source == fullScreenCheck.Get() || source == vsyncCheck.Get())
		{
			videoSettingDirty = true;
		}
	}
}

void SettingsMenu::OnLeave()
{
	ui->SetWidth(1280.0f);
	titleScreen->SendEvent(Settings::E_SETTINGSCHANGED);
	Menu::OnLeave();
}

SettingsMenu::~SettingsMenu()
{
}
