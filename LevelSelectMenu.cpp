#include "LevelSelectMenu.h"

#include <Urho3D/Graphics/Texture.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/UI/BorderImage.h>
#include <iostream>
#include "TitleScreen.h"
#include "GunPriest.h"

LevelSelectMenu::LevelSelectMenu(TitleScreen* ts, SharedPtr<Gameplay> gm) : Menu(ts, gm)
{
	layoutPath = "UI/titlemenus/levelselect.xml";
	//Find all possible level
	levelEntries = Vector<LevelEntry>();
	SharedPtr<XMLFile> levelInfo = SharedPtr<XMLFile>(cache->GetResource<XMLFile>("levelinfo.xml"));

	assert(levelInfo.Get() != nullptr);

	XPathQuery query("/levelinfo/level", "ResultSet");
	XPathResultSet results = query.Evaluate(levelInfo->GetRoot().GetChild("level"));
	for (int i = 0; i < results.Size(); i++)
	{
		LevelEntry fe;
		fe.filePath = results[i].GetAttribute("path");
		fe.levelName = results[i].GetAttribute("name");
		levelEntries.Push(fe);
	}
}

void LevelSelectMenu::OnEnter()
{
	GP::Menu::OnEnter();
	levelList = titleScreen->ourUI->GetChild("levelList", true);
	buttParent = levelList->GetChild("levelButtonParent", true);
	scrollBar = (ScrollBar*)levelList->GetChild("levelScroll", true);
	if (levelEntries.Size() > 10) 
	{
		scrollBar->SetRange((levelEntries.Size() - 10) / 20.0f);
	}
	else
	{
		scrollBar->SetRange(0.0f);
	}
	int counter = 0;
	for (Vector<LevelEntry>::Iterator i = levelEntries.Begin(); i != levelEntries.End(); ++i)
	{
		LevelEntry le = (LevelEntry)*i;
		Button* button = buttParent->CreateChild<Button>();
		button->LoadXML(cache->GetResource<XMLFile>("UI/titlemenus/levelbutton.xml")->GetRoot());
		button->SetTexture((Texture*)cache->GetResource<Texture2D>("Textures/UI.png"));
		button->SetPosition(0, 4 + (counter * 36));
		button->SetVar("filePath", le.filePath);
		Text* text = button->CreateChild<Text>();
		text->SetAlignment(HA_CENTER, VA_CENTER);
		text->SetFont("Fonts/Anonymous Pro.ttf", 16);
		text->SetText(le.levelName);
		text->SetEnabled(false);
		le.listItem = (UIElement*)button;
		counter += 1;
	}
}

void LevelSelectMenu::Update(float timeStep)
{
	if (scrollBar)
	{
		float y = scrollBar->GetValue();
		buttParent->SetPosition(0, -y * levelEntries.Size() * 36);
	}
}

void LevelSelectMenu::OnEvent(StringHash eventType, VariantMap& eventData)
{
	if (eventType == E_UIMOUSECLICKEND) 
	{
		UIElement* source = dynamic_cast<UIElement*>(eventData["Element"].GetPtr());
		if (source)
		{
			if (source->GetName() == "backButton")
			{
				titleScreen->SetMenu(titleScreen->titleMenu);
			}
			else if (source->GetName() == "levelButton")
			{
				const String path = source->GetVar("filePath").GetString();
				titleScreen->gunPriest->ChangeState(GunPriest::STATE_GAME);
				titleScreen->gunPriest->StartGame(path);
			}
		}
	}
}

LevelSelectMenu::~LevelSelectMenu()
{
	
}
