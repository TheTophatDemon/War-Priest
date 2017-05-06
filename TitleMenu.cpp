#include "TitleMenu.h"

#include "TitleScreen.h"
#include "GunPriest.h"

TitleMenu::TitleMenu(TitleScreen* ts, SharedPtr<Gameplay> gm) : Menu(ts, gm)
{
	layoutPath = "UI/titlemenus/titlescreen.xml";
}

void TitleMenu::OnEnter()
{
	Menu::OnEnter();
	resumeButton = (Button*)titleScreen->ourUI->GetChild("resumeGame", true);
	resumeButton->SetVisible(gameplay->initialized);
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
			titleScreen->gunPriest->ChangeState(GunPriest::STATE_GAME);
		}
		else if (source->GetName() == "startGame")
		{
			titleScreen->SetMenu(titleScreen->levelSelectMenu);
		}
	}
}

TitleMenu::~TitleMenu()
{
}
