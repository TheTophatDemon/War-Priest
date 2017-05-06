#include "Menu.h"

#include "TitleScreen.h"

Menu::Menu(TitleScreen* ts, SharedPtr<Gameplay> gm)
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
