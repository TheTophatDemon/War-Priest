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
	static void LoadSettings(Context* context);
	static void SaveSettings(Context* context);
	static void RevertSettings();

	static inline int GetBackwardKey() { return keyBackward; }
	static inline int GetForwardKey() { return keyForward; }
	static inline int GetRightKey() { return keyRight;  }
	static inline int GetLeftKey() { return keyLeft; }
	static inline int GetJumpKey() { return keyJump; }
	static inline int GetReviveKey() { return keyRevive; }
	static inline int GetSlideKey() { return keySlide; }
	static inline int GetResolutionX() { return xRes; };
	static inline int GetResolutionY() { return yRes; };
	static inline float GetMouseSensitivity() { return mouseSensitivity; }
	static inline float GetMusicVolume() { return musicVolume; }
	static inline float GetSoundVolume() { return soundVolume; }
	static inline float GetDifficulty() { return difficulty; }
	static inline bool IsMouseInverted() { return mouseInvert; }
	static inline bool IsBloodEnabled() { return bloodEnabled; }
	static inline bool IsFullScreen() { return fullScreen; }
	static inline bool IsVsync() { return vSync; }
	static inline bool AreGraphicsFast() { return fastGraphics; }

	static bool IsKeyDown(Input* input, int key);
	static float ScaleWithDifficulty(const float easyValue, const float hardValue, const float unholyValue);

	static StringHash E_SETTINGSCHANGED;
protected:
	static int keyBackward;
	static int keyForward;
	static int keyRight;
	static int keyLeft;
	static int keyJump;
	static int keyRevive;
	static int keySlide;
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

