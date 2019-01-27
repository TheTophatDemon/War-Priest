#include "LevelSelectMenu.h"

#include <Urho3D/Graphics/Texture.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/UI/Sprite.h>
#include <stdlib.h>
#include <iostream>
#include "TitleScreen.h"
#include "GunPriest.h"

#define LEVEL_INFO_FILE "levelinfo.xml"

const int LevelSelectMenu::LCF_UNLOCKED = 1;
const int LevelSelectMenu::LCF_BEATEN = 2;
const int LevelSelectMenu::LCF_CROSSGOTTEN = 4;

LevelSelectMenu::LevelSelectMenu(TitleScreen* ts, SharedPtr<Gameplay> gm) : Menu(ts, gm),
	animTimer(0.0f),
	crossCount(0),
	beatenCount(0)
{
	layoutPath = "UI/titlemenus/levelselect.xml";
	//Find all possible level
	levelEntries = Vector<LevelEntry>();
	levelInfo = SharedPtr<XMLFile>(cache->GetResource<XMLFile>(LEVEL_INFO_FILE));

	assert(levelInfo.Get() != nullptr);

	XPathQuery query("/levelinfo/level", "ResultSet");
	XPathResultSet results = query.Evaluate(levelInfo->GetRoot().GetChild("level"));
	for (int i = 0; i < results.Size(); i++)
	{
		LevelEntry fe;
		fe.filePath = results[i].GetAttribute("path");
		fe.levelName = results[i].GetAttribute("name");
		fe.completion = atoi(results[i].GetAttribute("completion").CString());
		fe.completionToll = atoi(results[i].GetAttribute("completiontoll").CString());
		fe.crossToll = atoi(results[i].GetAttribute("crosstoll").CString());
		levelEntries.Push(fe);
	}

	sprites = Vector<Sprite*>();
}

void LevelSelectMenu::SetLevelCompletionFlag(const String levelPath, const int flag, const bool val) 
{
	for (LevelEntry& le : levelEntries)
	{
		if (le.filePath == levelPath)
		{
			//Change flag
			if (val)
			{
				le.completion = le.completion | flag;
			}
			else 
			{
				if ((le.completion & flag) == flag) //This hasn't been tested yet, but nobody is likely to need it.
					le.completion -= flag;
			}
			//Update levelinfo xml structure
			levelInfo->GetRoot().RemoveChildren("level");
			for (LevelEntry& le : levelEntries)
			{
				XMLElement& elle = levelInfo->GetRoot().CreateChild("level");
				elle.SetString("name", le.levelName);
				elle.SetString("path", le.filePath);
				elle.SetInt("completion", le.completion);
				elle.SetInt("crosstoll", le.crossToll);
				elle.SetInt("completiontoll", le.completionToll);
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
	sprites.Clear();
	GP::Menu::OnEnter();
	levelList = titleScreen->ourUI->GetChildDynamicCast<ListView>("levelList", true);

	crossCount = 0;
	beatenCount = 0;
	int counter = 0;
	for (LevelEntry& le : levelEntries)
	{
		if (le.completion & LCF_BEATEN)
			beatenCount++;
		if (le.completion & LCF_CROSSGOTTEN)
			crossCount++;
	}
	
	for (LevelEntry& le : levelEntries)
	{
		//Unlock level if not done already
		if (!(le.completion & LCF_UNLOCKED) && beatenCount >= le.completionToll && crossCount >= le.crossToll)
		{
			SetLevelCompletionFlag(le.filePath, LCF_UNLOCKED, true);
		}

		//Generate button for each level
		if (le.completion & LCF_UNLOCKED) 
		{
			SharedPtr<UIElement> buttParent = SharedPtr<UIElement>(new UIElement(titleScreen->GetContext()));
			buttParent->SetStyleAuto(ui->GetRoot()->GetDefaultStyle());

			Button* button = buttParent->CreateChild<Button>();
			button->LoadXML(cache->GetResource<XMLFile>("UI/titlemenus/levelbutton.xml")->GetRoot());
			button->SetTexture((Texture*)cache->GetResource<Texture2D>("UI/ui.png"));
			button->SetVar("filePath", le.filePath);
			button->SetFocusMode(FM_NOTFOCUSABLE);

			if (le.completion & LCF_BEATEN)
			{
				button->SetColor(Color(0.5f, 1.0f, 0.5f));
			}

			Text* text = button->CreateChild<Text>();
			text->SetAlignment(HA_CENTER, VA_CENTER);
			text->SetFont("Fonts/Anonymous Pro.ttf", 20);
			text->SetText(le.levelName);
			text->SetEnabled(false);

			//Make spinning cross sprites
			if (le.completion & LCF_CROSSGOTTEN)
			{
				Sprite* sprite = button->CreateChild<Sprite>();
				sprite->SetSize(48, 48);
				sprite->SetAlignment(HA_LEFT, VA_TOP);
				sprite->SetPosition(0.0f, 0.0f);
				sprite->SetTexture(cache->GetResource<Texture2D>("UI/cross_icon.png"));
				sprite->SetImageRect(IntRect(0, 0, 48, 48));
				sprites.Push(sprite);
				Sprite* sprite2 = button->CreateChild<Sprite>();
				sprite2->SetSize(48, 48);
				sprite2->SetAlignment(HA_RIGHT, VA_TOP);
				sprite2->SetPosition(-48.0f, 0.0f);
				sprite2->SetTexture(cache->GetResource<Texture2D>("UI/cross_icon.png"));
				sprite2->SetImageRect(IntRect(0, 0, 48, 48));
				sprites.Push(sprite2);
			}

			buttParent->SetAlignment(HA_CENTER, VA_TOP);
			buttParent->SetSize(button->GetSize());
			buttParent->SetMinSize(button->GetSize());
			buttParent->SetMaxSize(button->GetSize());
			levelList->AddItem(buttParent);

			le.listItem = (UIElement*)button;
		}
	}
}

void LevelSelectMenu::Update(float timeStep)
{
	if (sprites.Size() > 0) 
	{
		//Animates the spinning cross sprites
		animTimer += timeStep;
		if (animTimer > 0.1f)
		{
			animTimer = 0.0f;
			Sprite* const baseSpr = sprites.At(0);
			Texture2D* tex = (Texture2D*)baseSpr->GetTexture();
			IntRect sourceRect = baseSpr->GetImageRect();
			const int width = sourceRect.Width();
			const int height = sourceRect.Height();
			sourceRect.right_ += width;
			sourceRect.left_ += width;
			if (sourceRect.right_ > tex->GetWidth())
			{
				sourceRect.right_ = width;
				sourceRect.left_ = 0;
				sourceRect.top_ += height;
				sourceRect.bottom_ += height;
			}
			if (sourceRect.bottom_ > tex->GetHeight())
			{
				sourceRect.bottom_ = height;
				sourceRect.top_ = 0;
				sourceRect.right_ = width;
				sourceRect.left_ = 0;
			}
			for (Sprite* spr : sprites)
			{
				spr->SetImageRect(sourceRect);
			}
		}
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
				//Mark that this level is being visited
				for (LevelEntry& le : levelEntries)
				{
					if (le.filePath == path)
					{
						le.visits++;
						break;
					}
				}
				titleScreen->gunPriest->ChangeState(GunPriest::STATE_GAME);
				titleScreen->gunPriest->StartGame(path);
			}
		}
	}
}

int LevelSelectMenu::GetNumberOfVisits(const String filePath)
{
	for (LevelEntry& le : levelEntries)
	{
		if (le.filePath == filePath)
		{
			return le.visits;
		}
	}
	std::cout << "LEVEL NOT FOUND IN GetNumberOfVisits()" << std::endl;
	return -1;
}

LevelSelectMenu::~LevelSelectMenu()
{
}
