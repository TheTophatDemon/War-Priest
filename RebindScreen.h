#pragma once

#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Slider.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/ListView.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/UI/ScrollView.h>
#include <Urho3D/UI/ScrollBar.h>

#include "Menu.h"
#include "GunPriest.h"
#include "Settings.h"

enum class BindingMode
{
	KEYB_MOUSE,
	JOYSTICK
};

class RebindScreen : public GP::Menu
{
	friend class SettingsMenu;

	URHO3D_OBJECT(RebindScreen, GP::Menu);
public:
	RebindScreen(Context* context, TitleScreen* titleScreen, SharedPtr<Gameplay> gm);
	virtual void OnEnter() override;
	virtual void Update(float timeStep) override;
	~RebindScreen();
protected:
	void OnEvent(StringHash eventType, VariantMap& eventData);
	void SyncControls();
	void SetRebinding(const bool mode);
	
	SharedPtr<Gameplay> gameplay;
	SharedPtr<Input> input;
	SharedPtr<UI> ui;
	SharedPtr<ResourceCache> cache;
	
	SharedPtr<Window> window;
	SharedPtr<ListView> panel;
	SharedPtr<Text> caption;
	Vector<SharedPtr<Button>> rebindButtons;
	Button* rebindButton;
	JoystickState prevJoyState;
	int lastJoyIndex;
	BindingMode bindingMode;
	//Timeout timer
	float rebindTimer = 0.0f;
	bool rebinding = false;
};

