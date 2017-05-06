#include "Menu.h"

#include "TitleScreen.h"

Menu::Menu(TitleScreen* ts, SharedPtr<Gameplay> gm)
{
	titleScreen = SharedPtr<TitleScreen>(ts);
	gameplay = gm;
	cache = titleScreen->GetSubsystem<ResourceCache>();
}

void Menu::DisableTexts()
{
	//Disable all texts so they don't interfere with button input
	PODVector<UIElement*> children;
	titleScreen->ourUI->GetChildren(children, true);
	for (PODVector<UIElement*>::Iterator i = children.Begin(); i != children.End(); ++i)
	{
		UIElement* element = (UIElement*)*i;
		//
		//
		//Replace with type check later
		//
		if (element->GetName().Empty())
		{
			element->SetEnabled(false);
		}
	}
}

Menu::~Menu()
{
}
