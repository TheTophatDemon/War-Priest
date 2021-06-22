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

#include <Urho3D/Core/Context.h>
#include <Urho3D/Input/Input.h>

using namespace Urho3D;

class SettingsMenu;

enum class BindingType : unsigned char
{
	KEY,
	MOUSE,
	JOYBUTT,
	JOYAXIS,
	JOYHAT,
	NONE
};

class Binding : public RefCounted //Universal Input type. Can be a keyboard key or a mouse button
{
public:
	Binding();
	Binding(String name, Input* input, BindingType type);
	String name;
	String verboseName;
	BindingType type;
	virtual float getValue() = 0;
	virtual bool valueChanged() = 0;
protected:
	Input* input;
};

class KeyBinding : public Binding
{
public:
	KeyBinding(const int keyCode, Input* input);
	virtual float getValue() override;
	virtual bool valueChanged() override;
	int keyCode;
};

class MouseBinding : public Binding
{
public:
	MouseBinding(const int button, Input* input);
	virtual float getValue() override;
	virtual bool valueChanged() override;
	int button;
};

class JoyButtBinding : public Binding
{
public:
	JoyButtBinding(const int joyIndex, const int button, Input* input);
	virtual float getValue() override;
	virtual bool valueChanged() override;
	int joyIndex;
	int button;
};

class JoyAxisBinding : public Binding
{
public:
	JoyAxisBinding(const int joyIndex, const int axis, const int sign, Input* input);
	virtual float getValue() override;
	virtual bool valueChanged() override;
	int joyIndex;
	int axis;
	int sign;
};

class JoyHatBinding : public Binding
{
public:
	JoyHatBinding(const int joyIndex, const int hat, const int direction, Input* input);
	virtual float getValue() override;
	virtual bool valueChanged() override;
	int joyIndex;
	int hat;
	int direction;
};

enum class ActionType
{
	FORWARD,
	BACK,
	LEFT,
	RIGHT,
	JUMP,
	REVIVE,
	SLIDE,
	CAM_RIGHT,
	CAM_LEFT,
	CAM_UP,
	CAM_DOWN,
	MENU,
	COUNT
};

class Action
{
public:
	Action(String name);
	float getValue();
	bool valueChanged();

	String name;
	//Binding for keyboard or mouse
	SharedPtr<Binding> binding;
	//Binding for joystick
	SharedPtr<Binding> joyBinding;
};

class Settings //Not to be confused with SettingsMenu.h
{
	friend class SettingsMenu;
	friend class RebindScreen;
public:
	static void LoadSettings(Context* context);
	static void SaveSettings(Context* context);
	static void RevertSettings(Context* context);
	
	static inline Action& GetAction(ActionType type) { return actions[static_cast<int>(type)]; }
	//These are declared in this class instead of Action in order to save typing when other classes are accessing this information
	static inline bool IsActionDown(ActionType type) { return actions[static_cast<int>(type)].getValue() > DEADZONE; }
	static inline float GetActionValue(ActionType type) { return actions[static_cast<int>(type)].getValue(); };
	static inline bool IsActionPressed(ActionType type) { return actions[static_cast<int>(type)].valueChanged(); };

	static inline int GetResolutionX() { return xRes; };
	static inline int GetResolutionY() { return yRes; };
	static inline float GetMouseSensitivity() { return mouseSensitivity; }
	static inline float GetMusicVolume() { return musicVolume; }
	static inline float GetSoundVolume() { return soundVolume; }
	static inline float GetDifficulty() { return difficulty; } //0.5: Easy, 1.0: Hard, 1.5: Unholy
	static inline bool IsMouseInverted() { return mouseInvert; }
	static inline bool IsBloodEnabled() { return bloodEnabled; }
	static inline bool IsFullScreen() { return fullScreen; }
	static inline bool IsVsync() { return vSync; }
	static inline bool AreGraphicsFast() { return fastGraphics; }

	//Linearly interpolates between three values depending on the game's difficulty scalar
	static inline float ScaleWithDifficulty(const float easyValue, const float hardValue, const float unholyValue)
	{
		float scaledDifficulty = GetDifficulty() - 0.5f;
		if (scaledDifficulty >= 0.5f)
		{
			scaledDifficulty -= 0.5f;
			scaledDifficulty *= 2.0f;
			return (hardValue * (1.0f - scaledDifficulty)) + (unholyValue * scaledDifficulty);
		}
		else
		{
			scaledDifficulty *= 2.0f;
			return (easyValue * (1.0f - scaledDifficulty)) + (hardValue * scaledDifficulty);
		}
	}

	static const constexpr float DEADZONE = 0.25f;
	static const constexpr int NUM_RESOLUTIONS = 6;
	static const int RES_X[];
	static const int RES_Y[];
	static const StringHash E_SETTINGSCHANGED;
	static const String GAMESETTINGS_PATH;
	static const float UNHOLY_THRESHOLD;
protected:
	static Action actions[];
	static int xRes;
	static int yRes;
	static float mouseSensitivity;
	static float musicVolume;
	static float soundVolume;
	static float difficulty;
	static bool mouseInvert;
	static bool bloodEnabled;
	static bool fullScreen;
	static bool vSync;
	static bool fastGraphics;
};

