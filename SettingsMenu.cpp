#include "SettingsMenu.h"
#include "TitleScreen.h"

#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Audio/Audio.h>
#include <iostream>

const Color SettingsMenu::selectedColor = Color(0.65f, 0.75f, 0.65f);
const Color SettingsMenu::unSelectedColor = Color(0.25f, 0.25f, 0.25f);

#define UPDATE_REBIND_BUTTON_TEXT(t, a) t.Substring(0, t.FindLast(":") + 1) + " " + a

SettingsMenu::SettingsMenu(TitleScreen* ts, SharedPtr<Gameplay> gm) : GP::Menu(ts, gm), 
	selectedRes(0), rebinding(false), rebindButton(nullptr), videoSettingDirty(false)
{
	layoutPath = "UI/titlemenus/settingsScreen.xml";
}

void SettingsMenu::OnEnter()
{
	Settings::LoadSettings(titleScreen->GetContext());
	GP::Menu::OnEnter();
	input = titleScreen->GetSubsystem<Input>();
	audio = titleScreen->GetSubsystem<Audio>();

	videoSettingDirty = false;
	
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

	//Set up rebinding window
	rebindWindow = dynamic_cast<Window*>(titleScreen->ourUI->LoadChildXML(cache->GetResource<XMLFile>("UI/titlemenus/rebindWindow.xml")->GetRoot()));
	rebindWindow->SetVisible(false);
	rebindWindow->SetPriority(100000);
	rebindPanel = rebindWindow->GetChildDynamicCast<BorderImage>("panel", true);
	rebindLabel = rebindWindow->GetChildDynamicCast<Text>("windowTitle", true);
	//Generate buttons for each input type
	rebindButtons = Vector<SharedPtr<Button>>();
	//Corresponding buttons must be in same order as Settings::inputs[]
	String inputLabels[] = {"Move Forward: ", "Move Backwards: ", "Move Left: ", "Move Right: ", "Jump: ", "Revive: ", "Slide: "};
	for (int i = 0; i < Settings::NUM_INPUTS; ++i)
	{
		Button* butt = dynamic_cast<Button*>(rebindPanel->LoadChildXML(cache->GetResource<XMLFile>("UI/titlemenus/rebindButton.xml")->GetRoot()));
		butt->GetChildDynamicCast<Text>("label", true)->SetText(inputLabels[i]);
		butt->SetPosition(butt->GetPosition().x_, butt->GetPosition().y_ + i * butt->GetHeight());
		butt->SetVar("Input ID", i);
		rebindButtons.Push(SharedPtr<Button>(butt));
	}

	//Add buttons for each resolution
	resolutionList = titleScreen->ourUI->GetChild("resolutionList", true);
	resolutionList->SetClipChildren(true);
	resolutionList->SetClipBorder(IntRect(4, 4, 4, 4));
	const int buttHeight = resolutionList->GetSize().y_ / Settings::NUM_RESOLUTIONS;
	for (int i = 0; i < Settings::NUM_RESOLUTIONS; ++i)
	{
		Button* butt = (Button*)resolutionList->CreateChild(Button::GetTypeStatic());
		butt->SetSize(resolutionList->GetSize().x_, buttHeight);
		butt->SetPosition(0, buttHeight*i);
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

void SettingsMenu::SyncControls() //Syncs the ui controls to the actual settings
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
	
	for (int i = 0; i < Settings::NUM_INPUTS; ++i)
	{
		Text* buttLabel = rebindButtons.At(i)->GetChildDynamicCast<Text>("label", true);
		buttLabel->SetText(UPDATE_REBIND_BUTTON_TEXT(buttLabel->GetText(), (*Settings::inputs[i])->name));
	}

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

void SettingsMenu::Update(float timeStep)
{
	//Sync settings to UI elements
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

	//Update volumes
	audio->SetMasterGain("GAMEPLAY", Settings::GetSoundVolume());
	audio->SetMasterGain("MUSIC", Settings::GetMusicVolume());
}

void SettingsMenu::OnEvent(StringHash eventType, VariantMap& eventData)
{
	if (!rebinding)
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
			else if (source->GetParent() == rebindPanel)
			{
				if (source->GetName() == "rebindButton")
				{
					rebinding = true;
					rebindButton = source;
					rebindLabel->SetText("Press the button to be assigned");
				}
			}
			else if (source->GetParent() == rebindWindow)
			{
				if (source->GetName() == "closeButton")
				{
					rebindWindow->SetVisible(false);
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
					Settings::RevertSettings(titleScreen->GetContext());
					SyncControls();
				}
				else if (source->GetName() == "controlsButton")
				{
					rebindWindow->SetVisible(true);
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
	else
	{
		if (eventType == E_UIMOUSECLICKEND || eventType == E_KEYDOWN) 
		{
			const int id = rebindButton->GetVar("Input ID").GetInt();
			if (eventType == E_UIMOUSECLICKEND)
			{
				const int button = eventData["Button"].GetInt();
				(*Settings::inputs[id]) = SharedPtr<UInput>(new MouseInput(button, input));
			}
			else if (eventType == E_KEYDOWN)
			{
				const int key = eventData["Key"].GetInt();
				(*Settings::inputs[id]) = SharedPtr<UInput>(new KeyInput(key, input));
			}
			rebinding = false;
			rebindLabel->SetText("Click on an input to change it");
			Text* buttLabel = rebindButton->GetChildDynamicCast<Text>("label", true);
			buttLabel->SetText(UPDATE_REBIND_BUTTON_TEXT(buttLabel->GetText(), (*Settings::inputs[id])->name));
			rebindButton = nullptr;
		}
	}
}

void SettingsMenu::OnLeave()
{
	ui->SetWidth(1280.0f);
	titleScreen->SendEvent(Settings::E_SETTINGSCHANGED, VariantMap());
}

SettingsMenu::~SettingsMenu()
{
}
