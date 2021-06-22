#include "TitleMenu.h"

#include <Urho3D/Engine/Engine.h>

#include "TitleScreen.h"
#include "GunPriest.h"

TitleMenu::TitleMenu(Context* context, TitleScreen* ts, SharedPtr<Gameplay> gm) : Menu(context, ts, gm)
{
	layoutPath = "UI/titlemenus/titlescreen.xml";
	SubscribeToEvent(E_UIMOUSECLICKEND, URHO3D_HANDLER(TitleMenu, OnEvent));
}

void TitleMenu::OnEnter()
{
	GP::Menu::OnEnter();
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

void TitleMenu::OnEvent(StringHash eventType, VariantMap& eventData)
{
	if (eventType == E_UIMOUSECLICKEND) 
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
			else if (source->GetName() == "settings")
			{
				titleScreen->SetMenu(titleScreen->settingsMenu);
			}
			else if (source->GetName() == "exitGame")
			{
				Engine* engine = titleScreen->GetSubsystem<Engine>();
				engine->Exit();
			}
		}
	}
}

TitleMenu::~TitleMenu()
{
}
