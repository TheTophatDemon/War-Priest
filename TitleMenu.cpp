#include "TitleMenu.h"

#include "TitleScreen.h"

TitleMenu::TitleMenu(TitleScreen* ts, SharedPtr<Gameplay> gm) : Menu(ts, gm)
{
	
}

void TitleMenu::OnEnter()
{
	titleScreen->ourUI->LoadXML(cache->GetResource<XMLFile>("UI/titlemenus/titlescreen.xml")->GetRoot()); //'Twas null!
	titleScreen->ourUI->GetName();
	resumeButton = (Button*)titleScreen->ourUI->GetChild("resumeGame", true);
	resumeButton->SetVisible(gameplay->initialized);
	DisableTexts();
}

void TitleMenu::Update(float timeStep)
{
	if (gameplay->initialized && !resumeButton->IsVisible())
	{
		resumeButton->SetVisible(true);
	}
}

void TitleMenu::OnClick(StringHash eventType, VariantMap& eventData)
{
	UIElement* source = (UIElement*)eventData["Element"].GetPtr();
	if (source)
	{
		if (source->GetName() == "resumeGame")
		{
			titleScreen->gotoGame = true;
		}
		else if (source->GetName() == "startGame")
		{
			gameplay->initialized = false;
			titleScreen->gotoGame = true;
		}
	}
}

TitleMenu::~TitleMenu()
{
}
