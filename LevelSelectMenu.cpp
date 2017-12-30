#include "LevelSelectMenu.h"

#include <Urho3D/Graphics/Texture.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/File.h>
#include <stdlib.h>
#include <iostream>
#include "TitleScreen.h"
#include "GunPriest.h"

#define LEVEL_INFO_FILE "levelinfo.xml"

int LevelSelectMenu::LCF_UNLOCKED = 1;
int LevelSelectMenu::LCF_BEATEN = 2;
int LevelSelectMenu::LCF_CROSSGOTTEN = 4;

LevelSelectMenu::LevelSelectMenu(TitleScreen* ts, SharedPtr<Gameplay> gm) : Menu(ts, gm)
{
	layoutPath = "UI/titlemenus/levelselect.xml";
	//Find all possible level
	levelEntries = Vector<LevelEntry*>();
	levelInfo = SharedPtr<XMLFile>(cache->GetResource<XMLFile>(LEVEL_INFO_FILE));

	assert(levelInfo.Get() != nullptr);

	XPathQuery query("/levelinfo/level", "ResultSet");
	XPathResultSet results = query.Evaluate(levelInfo->GetRoot().GetChild("level"));
	for (int i = 0; i < results.Size(); i++)
	{
		LevelEntry* fe = new LevelEntry();
		fe->filePath = results[i].GetAttribute("path");
		fe->levelName = results[i].GetAttribute("name");
		fe->completion = atoi(results[i].GetAttribute("completion").CString());
		levelEntries.Push(fe);
	}
}

void LevelSelectMenu::SetLevelCompletionFlag(const String levelPath, const int flag, const bool val) 
{
	for (Vector<LevelEntry*>::Iterator i = levelEntries.Begin(); i != levelEntries.End(); ++i)
	{
		LevelEntry* le = (LevelEntry*)*i;
		if (le->filePath == levelPath)
		{
			//Change flag
			if (val)
			{
				if (!(le->completion & flag))
					le->completion += flag;
			}
			else
			{
				if (le->completion & flag)
					le->completion -= flag;
			}
			//Update levelinfo xml structure
			levelInfo->GetRoot().RemoveChildren("level");
			for (LevelEntry* le : levelEntries)
			{
				XMLElement& elle = levelInfo->GetRoot().CreateChild("level");
				elle.SetString("name", le->levelName);
				elle.SetString("path", le->filePath);
				elle.SetInt("completion", le->completion);
			}
			//Save to levelinfo.xml
			const FileSystem* const fileSystem = titleScreen->GetContext()->GetSubsystem<FileSystem>();
			File* const file = new File(titleScreen->GetContext());
			const bool succ = file->Open(fileSystem->GetProgramDir() + "/Data/" + LEVEL_INFO_FILE, FILE_WRITE);
			if (succ)
			{
				const bool succ_again = levelInfo->Save(*file);
				if (!succ_again)
					std::cout << "UNABLE TO SAVE LEVELINFO INTO LEVELINFO.XML" << std::endl;
				file->Close();
			}
			else
			{
				std::cout << "UNABLE TO OPEN LEVELINFO.XML FOR WRITING" << std::endl;
			}
			return;
		}
	}
	std::cout << "SetLevelCompletion found no corresponding level entry." << std::endl;
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

	int numBeaten = 0;
	int counter = 0;
	for (LevelEntry* le : levelEntries)
	{
		if (le->completion & LCF_BEATEN)
			numBeaten += 1;
	}
	LevelEntry* lastEntry = nullptr;
	for (Vector<LevelEntry*>::Iterator i = levelEntries.Begin(); i != levelEntries.End(); ++i)
	{
		LevelEntry* le = (LevelEntry*)*i;
		if (lastEntry) 
		{
			if (lastEntry->completion & LCF_BEATEN && !(le->completion & LCF_UNLOCKED))
			{
				SetLevelCompletionFlag(le->filePath, LCF_UNLOCKED, true);
			}
		}

		if (le->completion & LCF_UNLOCKED) 
		{
			Button* button = buttParent->CreateChild<Button>();
			button->LoadXML(cache->GetResource<XMLFile>("UI/titlemenus/levelbutton.xml")->GetRoot());
			button->SetTexture((Texture*)cache->GetResource<Texture2D>("Textures/UI.png"));
			button->SetPosition(0, 4 + (counter * 36));
			button->SetVar("filePath", le->filePath);
			if (le->completion & LCF_BEATEN)
			{
				button->SetColor(Color(0.0f, 1.0f, 0.0f));
			}

			Text* text = button->CreateChild<Text>();
			text->SetAlignment(HA_CENTER, VA_CENTER);
			text->SetFont("Fonts/Anonymous Pro.ttf", 16);
			text->SetText(le->levelName);
			text->SetEnabled(false);

			le->listItem = (UIElement*)button;
		}

		lastEntry = le;
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
	for (LevelEntry* le : levelEntries)
	{
		delete le;
	}
}
