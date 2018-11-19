#pragma once

#include <Urho3D/Core/Context.h>
#include <Urho3D/Input/Input.h>

using namespace Urho3D;

class SettingsMenu;

class UInput : public RefCounted //Universal Input type. Can be a keyboard key or a mouse button
{
public:
	UInput();
	UInput(String name, Input* input);
	String name;
	virtual bool isDown() = 0;
	virtual bool isPressed() = 0;
protected:
	Input* input;
};
class KeyInput : public UInput
{
public:
	KeyInput(const int keyCode, Input* input);
	virtual bool isDown() override;
	virtual bool isPressed() override;
	int keyCode;
};
class MouseInput : public UInput
{
public:
	MouseInput(const int button, Input* input);
	virtual bool isDown() override;
	virtual bool isPressed() override;
	int button;
};

class Settings //Not to be confused with SettingsMenu.h
{
	friend class SettingsMenu;
public:
	static void LoadSettings(Context* context);
	static void SaveSettings(Context* context);
	static void RevertSettings(Context* context);
	
	static inline UInput* GetBackwardKey() { return keyBackward; }
	static inline UInput* GetForwardKey() { return keyForward; }
	static inline UInput* GetRightKey() { return keyRight;  }
	static inline UInput* GetLeftKey() { return keyLeft; }
	static inline UInput* GetJumpKey() { return keyJump; }
	static inline UInput* GetReviveKey() { return keyRevive; }
	static inline UInput* GetSlideKey() { return keySlide; }
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

	static const constexpr int NUM_INPUTS = 7;
	static const constexpr int NUM_RESOLUTIONS = 6;
	static const int RES_X[];
	static const int RES_Y[];
	static StringHash E_SETTINGSCHANGED;
	static String GAMESETTINGS_PATH;
	static const float UNHOLY_THRESHOLD;
protected:
	static SharedPtr<UInput>* inputs[];
	static SharedPtr<UInput> keyForward;
	static SharedPtr<UInput> keyBackward;
	static SharedPtr<UInput> keyLeft;
	static SharedPtr<UInput> keyRight;
	static SharedPtr<UInput> keyJump;
	static SharedPtr<UInput> keyRevive;
	static SharedPtr<UInput> keySlide;
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

