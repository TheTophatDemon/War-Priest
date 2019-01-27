#pragma once

#include <Urho3D/UI/ListView.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/ScrollView.h>
#include <Urho3D/UI/ScrollBar.h>
#include <Urho3D/Container/Vector.h>
#include "Menu.h"

struct LevelEntry
{
	String filePath;
	String levelName;
	SharedPtr<UIElement> listItem;
	int completion;
	int visits = 0; //Number of time this level has been played this session
	int completionToll = 0; //Number of beaten levels required to unlock this level
	int crossToll = 0; //Number of crosses required to unlock this level
};

class LevelSelectMenu : public GP::Menu
{
public:
	static const int LCF_UNLOCKED;
	static const int LCF_BEATEN;
	static const int LCF_CROSSGOTTEN;

	LevelSelectMenu(TitleScreen* ts, SharedPtr<Gameplay> gm);
	virtual void OnEnter();
	virtual void Update(float timeStep);
	virtual void OnEvent(StringHash eventType, VariantMap& eventData);
	void SetLevelCompletionFlag(const String levelPath, const int flag, const bool val);
	int GetNumberOfVisits(const String levelpath);
	inline int GetCrossCount() const 
	{
		return crossCount;
	};
	inline int GetNumLevelsBeaten() const
	{
		return beatenCount;
	};
	~LevelSelectMenu();
protected:
	SharedPtr<XMLFile> levelInfo;
	SharedPtr<ListView> levelList;
	Vector<LevelEntry> levelEntries;
	Vector<Sprite*> sprites;
	int crossCount;
	int beatenCount;
	float animTimer;
};

