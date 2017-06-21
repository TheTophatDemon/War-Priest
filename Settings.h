#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Input/Input.h>
#include "SettingsMenu.h"

using namespace Urho3D;

class Settings //Not to be confused with SettingsMenu.h
{
	friend class SettingsMenu;
public:
	static String GAMESETTINGS_PATH;
	static String VIDEOSETTINGS_PATH;
	static void LoadSettings(Context* context);
	static void SaveSettings(Context* context);
	static void RevertSettings();

	static int GetBackwardKey() { return keyBackward; }
	static int GetForwardKey() { return keyForward; }
	static int GetRightKey() { return keyRight;  }
	static int GetLeftKey() { return keyLeft; }
	static int GetJumpKey() { return keyJump; }
	static int GetReviveKey() { return keyRevive; }
	static int GetSlideKey() { return keySlide; }
	static float GetMouseSensitivity() { return mouseSensitivity; }
	static float GetMusicVolume() { return musicVolume; }
	static float GetSoundVolume() { return soundVolume; }
	static bool AreGraphicsFast() { return fastGraphics; }
	static bool IsMouseInverted() { return mouseInvert; }
	static bool IsBloodEnabled() { return bloodEnabled; }
	static bool IsFullScreen() { return fullScreen; }
	static bool IsVsync() { return vSync; }
protected:
	static int keyBackward;
	static int keyForward;
	static int keyRight;
	static int keyLeft;
	static int keyJump;
	static int keyRevive;
	static int keySlide;
	static float mouseSensitivity;
	static float musicVolume;
	static float soundVolume;
	static bool fastGraphics;
	static bool mouseInvert;
	static bool bloodEnabled;
	static bool fullScreen;
	static bool vSync;
};

