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
};

class LevelSelectMenu : public Menu
{
public:
	LevelSelectMenu(TitleScreen* ts, SharedPtr<Gameplay> gm);
	virtual void OnEnter();
	virtual void Update(float timeStep);
	virtual void OnEvent(StringHash eventType, VariantMap& eventData);
	~LevelSelectMenu();
protected:
	SharedPtr<UIElement> levelList;
	SharedPtr<ScrollBar> scrollBar;
	SharedPtr<UIElement> buttParent;
	Vector<LevelEntry> levelEntries;
};

