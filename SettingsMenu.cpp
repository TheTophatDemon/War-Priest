#include "SettingsMenu.h"
#include "TitleScreen.h"
#include "Settings.h"

#include <iostream>

SettingsMenu::SettingsMenu(TitleScreen* ts, SharedPtr<Gameplay> gm) : Menu(ts, gm)
{
	layoutPath = "UI/titlemenus/settingsScreen.xml";
}

void SettingsMenu::OnEnter()
{
	Menu::OnEnter();
}

void SettingsMenu::Update(float timeStep)
{
	
}

void SettingsMenu::OnClick(StringHash eventType, VariantMap& eventData)
{
	UIElement* source = dynamic_cast<UIElement*>(eventData["Element"].GetPtr());
	if (source)
	{
		if (source->GetName() == "cancelButton")
		{
			titleScreen->SetMenu(titleScreen->titleMenu);
		}
		else if (source->GetName() == "confirmButton")
		{
			Settings::SaveSettings(titleScreen->GetContext());
			titleScreen->SetMenu(titleScreen->titleMenu);
		}
		else if (source->GetName() == "revertButton")
		{
			Settings::RevertSettings();
			OnEnter(); //So it'll update the sliders n' whatnot.
		}
	}
}

SettingsMenu::~SettingsMenu()
{
}
