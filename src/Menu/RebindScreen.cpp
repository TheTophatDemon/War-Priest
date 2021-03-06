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

#include "RebindScreen.h"

#define UPDATE_REBIND_BUTTON_TEXT(t, a) t.Substring(0, t.FindLast(":") + 1) + " " + a

RebindScreen::RebindScreen(Context* context, TitleScreen* titleScreen, SharedPtr<Gameplay> gm) : Menu(context, titleScreen, gm),
	rebinding(false), rebindButton(nullptr), 
	bindingMode(BindingMode::KEYB_MOUSE), rebindTimer(0.0f), lastJoyIndex(0)
{
	layoutPath = "UI/titlemenus/rebindScreen.xml";

	gameplay = gm;
	input = GetSubsystem<Input>();
	ui = GetSubsystem<UI>();
	cache = GetSubsystem<ResourceCache>();
}

void RebindScreen::OnEnter()
{
	GP::Menu::OnEnter();

	//Set up rebinding window
	window = dynamic_cast<Window*>(titleScreen->ourUI->GetChild("rebindWindow", true));
	caption = window->GetChildDynamicCast<Text>("windowTitle", true);
	panel = window->GetChildDynamicCast<ListView>("panel", true);
	//Hacking the UI Style so that the ListView items don't show over the panel borders reveals the horizontal scroll bar
	//Other code messes with its visibility state, so it must be hidden indirectly
	panel->GetHorizontalScrollBar()->SetClipChildren(true);
	panel->GetHorizontalScrollBar()->SetClipBorder(IntRect(6000, 6000, 6000, 6000));
	panel->SetFocus(true);
	
	//Generate buttons for each input type
	rebindButtons = Vector<SharedPtr<Button>>();
	for (int i = 0; i < static_cast<int>(ActionType::COUNT); ++i)
	{
		//Have to add a proxy UIElement for each button or the ListView will not work right
		SharedPtr<UIElement> buttParent = SharedPtr<UIElement>(new UIElement(titleScreen->GetContext()));
		buttParent->SetStyleAuto(ui->GetRoot()->GetDefaultStyle());

		Button* butt = dynamic_cast<Button*>(buttParent->LoadChildXML(cache->GetResource<XMLFile>("UI/titlemenus/rebindButton.xml")->GetRoot()));
		butt->GetChildDynamicCast<Text>("label", true)->SetText(Settings::actions[i].name + ":");
		butt->SetVar("Input ID", i);
		butt->SetFocusMode(FM_FOCUSABLE);
		rebindButtons.Push(SharedPtr<Button>(butt));

		buttParent->SetAlignment(HA_CENTER, VA_TOP);
		buttParent->SetSize(butt->GetSize());
		buttParent->SetMinSize(butt->GetSize());
		buttParent->SetMaxSize(butt->GetSize());
		panel->AddItem(buttParent);
	}

	SubscribeToEvent(E_UIMOUSECLICKEND, URHO3D_HANDLER(RebindScreen, OnEvent));
	SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(RebindScreen, OnEvent));
	SubscribeToEvent(E_JOYSTICKBUTTONDOWN, URHO3D_HANDLER(RebindScreen, OnEvent));
	SubscribeToEvent(E_JOYSTICKAXISMOVE, URHO3D_HANDLER(RebindScreen, OnEvent));
	SubscribeToEvent(E_JOYSTICKHATMOVE, URHO3D_HANDLER(RebindScreen, OnEvent));

	SyncControls();
}

void RebindScreen::SyncControls()
{
	for (int i = 0; i < static_cast<int>(ActionType::COUNT); ++i)
	{
		Text* buttLabel = rebindButtons.At(i)->GetChildDynamicCast<Text>("label", true);
		switch (bindingMode)
		{
		case BindingMode::KEYB_MOUSE:
			if (Settings::actions[i].binding.NotNull())
				buttLabel->SetText(UPDATE_REBIND_BUTTON_TEXT(buttLabel->GetText(), Settings::actions[i].binding->name));
			else
				buttLabel->SetText(UPDATE_REBIND_BUTTON_TEXT(buttLabel->GetText(), "None"));
			break;
		case BindingMode::JOYSTICK:
			if (Settings::actions[i].joyBinding.NotNull())
				buttLabel->SetText(UPDATE_REBIND_BUTTON_TEXT(buttLabel->GetText(), Settings::actions[i].joyBinding->name));
			else
				buttLabel->SetText(UPDATE_REBIND_BUTTON_TEXT(buttLabel->GetText(), "None"));
			break;
		}
	}

	switch (bindingMode)
	{
	case BindingMode::KEYB_MOUSE:
		if (rebinding)
			caption->SetText("Press a key or mouse button");
		else
			caption->SetText("Click an action to assign to keyboard/mouse buttons.");
		break;
	case BindingMode::JOYSTICK:
		if (rebinding)
			caption->SetText("Press a button or move an axis");
		else
			caption->SetText("Click an action to assign to joystick axes/buttons.");
		if (input->GetNumJoysticks() == 0)
			caption->SetText("No joystick connected. Try restarting the game.");
		break;
	default:
		caption->SetText("Invalid binding mode!?");
		break;
	}
}

void RebindScreen::Update(float timeStep)
{
	//Rebinding timeout. 
	//Makes sure you can get out of the menu if you try to make a joystick binding without a joystick.
	if (rebinding)
	{
		rebindTimer += timeStep;
		if (rebindTimer > 10.0f)
		{
			rebindTimer = 0.0f;
			SetRebinding(false);
		}
	}
	else
	{
		rebindTimer = 0.0f;
	}
	
	//Check axes for binding here
	if (rebinding && bindingMode == BindingMode::JOYSTICK)
	{
		const int inputId = rebindButton->GetVar("Input ID").GetInt();
		
		for (int joyNum = 0; joyNum < input->GetNumJoysticks(); ++joyNum)
		{
			JoystickState* joy = input->GetJoystickByIndex(joyNum);
			assert(joy);

			//Axis Movements
			float maxDelta = -FLT_MAX;
			int probableAxis = -1;
			for (int axis = 0; axis < joy->GetNumAxes(); ++axis)
			{
				const float value = joy->GetAxisPosition(axis);
				const float delta = value - prevJoyState.GetAxisPosition(axis);
				if (fabs(delta) > 0.2f && delta > maxDelta)
				{
					maxDelta = delta;
					probableAxis = axis;
				}
			}
			//The axis that moves the most is bound
			if (probableAxis >= 0)
			{
				Settings::actions[inputId].joyBinding = new JoyAxisBinding(joyNum, probableAxis, Sign(maxDelta), input);
				SetRebinding(false);
				prevJoyState = *joy;
			}
		}
	}

	JoystickState* joy = input->GetJoystick(prevJoyState.joystickID_);
	if (joy) prevJoyState = *joy;
	else prevJoyState = JoystickState();
}

void RebindScreen::OnEvent(StringHash eventType, VariantMap& eventData)
{
	if (!rebinding && eventType == E_UIMOUSECLICKEND)
	{
		Button* source = dynamic_cast<Button*>(eventData["Element"].GetPtr());
		if (!source) return;
		if (source->GetName() == "rebindButton")
		{
			rebindButton = source;
			SetRebinding(true);
		}
		else if (source->GetName() == "bindKeyboardButton")
		{
			bindingMode = BindingMode::KEYB_MOUSE;
			SyncControls();
		}
		else if (source->GetName() == "bindJoystickButton")
		{
			bindingMode = BindingMode::JOYSTICK;
			SyncControls();
		}
	}
	else if (rebinding)
	{
		const int id = rebindButton->GetVar("Input ID").GetInt();
		if (bindingMode == BindingMode::KEYB_MOUSE)
		{
			if (eventType == E_UIMOUSECLICKEND)
			{
				const int button = eventData["Button"].GetInt();
				Settings::actions[id].binding = new MouseBinding(button, input);
				SetRebinding(false);
			}
			else if (eventType == E_KEYDOWN)
			{
				const int key = eventData["Key"].GetInt();
				Settings::actions[id].binding = new KeyBinding(key, input);
				SetRebinding(false);
			}
		}
		else if (bindingMode == BindingMode::JOYSTICK)
		{
			//Axes are tested in RebindScreen::Update() for expediency.
			JoystickState* joy = nullptr;
			if (eventType == E_JOYSTICKBUTTONDOWN)
			{
				const int index = eventData["JoystickID"].GetInt();
				const int button = eventData["Button"].GetInt();
				Settings::actions[id].joyBinding = new JoyButtBinding(index, button, input);
				SetRebinding(false);
				joy = input->GetJoystickByIndex(index);
			}
			else if (eventType == E_JOYSTICKHATMOVE)
			{
				const int index = eventData["JoystickID"].GetInt();
				lastJoyIndex = index;
				const int hat = eventData["Button"].GetInt();
				const int position = eventData["Position"].GetInt();
				Settings::actions[id].joyBinding = new JoyHatBinding(index, hat, position, input);
				SetRebinding(false);
				joy = input->GetJoystickByIndex(index);
			}
			//When a joystick event has occurred, save its state in case a comparison must be made...
			if (!rebinding && joy)
			{
				prevJoyState = *joy;
			}
		}
	}
}

void RebindScreen::SetRebinding(const bool mode)
{
	if (mode == rebinding) return;
	rebinding = mode;
	rebindTimer = 0.0f;
	if (mode)
	{
		rebindButton->SetFocus(true);
		SyncControls();
	}
	else
	{
		rebindButton = nullptr;
		panel->SetFocus(true);
		SyncControls();
	}
}

RebindScreen::~RebindScreen()
{
}
