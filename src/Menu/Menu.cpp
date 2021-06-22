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

#include "Menu.h"

#include "../TitleScreen.h"

using namespace GP;

Menu::Menu(Context* context, TitleScreen* ts, SharedPtr<Gameplay> gm) : Object(context)
{
	titleScreen = SharedPtr<TitleScreen>(ts);
	gameplay = gm;
	cache = titleScreen->GetSubsystem<ResourceCache>();
	ui = titleScreen->GetSubsystem<UI>();
}

void Menu::OnEnter()
{
	titleScreen->ourUI->RemoveAllChildren();
	titleScreen->ourUI->LoadXML(cache->GetResource<XMLFile>(layoutPath)->GetRoot());
	DisableTexts();
}

void Menu::OnLeave()
{
	titleScreen->ourUI->SetEnabledRecursive(false); //Makes sure unparented children don't fire events
	titleScreen->ourUI->SetEnabled(true);
	titleScreen->ourUI->RemoveAllChildren();
}

void Menu::Update(float timeStep){}

void Menu::DisableTexts()
{
	//Disable all texts so they don't interfere with button input
	PODVector<UIElement*> children;
	titleScreen->ourUI->GetChildren(children, true);
	for (PODVector<UIElement*>::Iterator i = children.Begin(); i != children.End(); ++i)
	{
		UIElement* element = (UIElement*)*i;
		if (element->GetTypeName() == "Text")
		{
			element->SetEnabled(false);
		}
	}
}

Menu::~Menu()
{
}
