/*
Copyright (C) 2021 Alexander Lunsford

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "TitleMenu.h"

#include <Urho3D/Engine/Engine.h>

#include "../TitleScreen.h"
#include "../GunPriest.h"

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
