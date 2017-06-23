#pragma once

#include "Menu.h"

class TitleMenu : public Menu
{
public:
	TitleMenu(TitleScreen* ts, SharedPtr<Gameplay> gm);
	virtual void OnEnter();
	virtual void Update(float timeStep);
	virtual void OnEvent(StringHash eventType, VariantMap& eventData);
	~TitleMenu();
protected:
	SharedPtr<Button> resumeButton;
};

