#pragma once

#include "Menu.h"

class TitleMenu : public GP::Menu
{
	URHO3D_OBJECT(TitleMenu, GP::Menu);
public:
	TitleMenu(Context* context, TitleScreen* ts, SharedPtr<Gameplay> gm);
	virtual void OnEnter() override;
	virtual void Update(float timeStep) override;
	~TitleMenu();
protected:
	void OnEvent(StringHash eventType, VariantMap& eventData);
	SharedPtr<Button> resumeButton;
};

