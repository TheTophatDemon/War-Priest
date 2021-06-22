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

	LevelSelectMenu(Context* context, TitleScreen* ts, SharedPtr<Gameplay> gm);
	virtual void OnEnter() override;
	virtual void Update(float timeStep) override;
	void SetLevelCompletionFlag(const String levelPath, const int flag, const bool val);
	int GetNumberOfVisits(const String levelpath);
	inline int GetRelicCount() const 
	{
		return crossCount;
	};
	inline int GetNumLevelsBeaten() const
	{
		return beatenCount;
	};
	~LevelSelectMenu();
protected:
	void OnEvent(StringHash eventType, VariantMap& eventData);
	
	SharedPtr<XMLFile> levelInfo;
	SharedPtr<ListView> levelList;
	Vector<LevelEntry> levelEntries;
	Vector<Sprite*> sprites;
	int crossCount;
	int beatenCount;
	float animTimer;
};

