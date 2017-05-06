#include "LevelSelectMenu.h"

#include "TitleScreen.h"

LevelSelectMenu::LevelSelectMenu(TitleScreen* ts, SharedPtr<Gameplay> gm) : Menu(ts, gm)
{
	layoutPath = "UI/titlemenus/levelselect.xml";
}

void LevelSelectMenu::OnEnter()
{
	Menu::OnEnter();
}

void LevelSelectMenu::Update(float timeStep)
{
	
}

void LevelSelectMenu::OnClick(StringHash eventType, VariantMap& eventData)
{
	UIElement* source = (UIElement*)eventData["Element"].GetPtr();
	if (source)
	{
		if (source->GetName() == "backButton")
		{
			titleScreen->SetMenu(titleScreen->titleMenu);
		}
	}
}

LevelSelectMenu::~LevelSelectMenu()
{
}
