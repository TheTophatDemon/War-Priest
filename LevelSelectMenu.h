#pragma once

#include "Menu.h"

class LevelSelectMenu : public Menu
{
public:
	LevelSelectMenu(TitleScreen* ts, SharedPtr<Gameplay> gm);
	virtual void OnEnter();
	virtual void Update(float timeStep);
	virtual void OnClick(StringHash eventType, VariantMap& eventData);
	~LevelSelectMenu();
};

