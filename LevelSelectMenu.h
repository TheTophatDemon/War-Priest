#pragma once

#include <Urho3D/UI/ListView.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/ScrollView.h>
#include <Urho3D/UI/ScrollBar.h>
#include "Menu.h"

struct LevelEntry
{
	String filePath;
	String levelName;
	SharedPtr<UIElement> listItem;
	int completion;
};

class LevelSelectMenu : public GP::Menu
{
public:
	static int LCF_UNLOCKED;
	static int LCF_BEATEN;
	static int LCF_CROSSGOTTEN;

	LevelSelectMenu(TitleScreen* ts, SharedPtr<Gameplay> gm);
	virtual void OnEnter();
	virtual void Update(float timeStep);
	virtual void OnEvent(StringHash eventType, VariantMap& eventData);
	void SetLevelCompletionFlag(const String levelPath, const int flag, const bool val);
	~LevelSelectMenu();
protected:
	SharedPtr<XMLFile> levelInfo;
	SharedPtr<UIElement> levelList;
	SharedPtr<UIElement> buttParent;
	SharedPtr<ScrollBar> scrollBar;
	Vector<LevelEntry*> levelEntries;
};

