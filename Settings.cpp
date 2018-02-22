#include "Settings.h"
#include <iostream>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/File.h>

String Settings::GAMESETTINGS_PATH = "/Data/gamesettings.bin";
StringHash Settings::E_SETTINGSCHANGED = StringHash("SettingsChanged");

#define DF_FASTGRAPHICS false
#define DF_MOUSEINVERT false
#define DF_BLOOD true
#define DF_FULLSCREEN false
#define DF_VSYNC false

#define DF_MOUSESENS 0.25f
#define DF_MUSVOL 0.5f
#define DF_SNDVOL 0.5f
#define DF_DIFF 1.0f

#define DF_KBACK 115
#define DF_KFOR 119
#define DF_KLEF 97
#define DF_KRIG 100
#define DF_KJUMP 32
#define DF_KREV 1073741895
#define DF_KSLIDE 1073742055

#define DF_XRES 1280
#define DF_YRES 720

bool Settings::fastGraphics = DF_FASTGRAPHICS;
bool Settings::mouseInvert = DF_MOUSEINVERT;
bool Settings::bloodEnabled = DF_BLOOD;
bool Settings::fullScreen = DF_FULLSCREEN;
bool Settings::vSync = DF_VSYNC;

float Settings::mouseSensitivity = DF_MOUSESENS;
float Settings::musicVolume = DF_MUSVOL;
float Settings::soundVolume = DF_SNDVOL;
float Settings::difficulty = DF_DIFF;

int Settings::keyBackward = DF_KBACK;
int Settings::keyForward = DF_KFOR;
int Settings::keyLeft = DF_KLEF;
int Settings::keyRight = DF_KRIG;
int Settings::keyJump = DF_KJUMP;
int Settings::keyRevive = DF_KREV;
int Settings::keySlide = DF_KSLIDE;

int Settings::xRes = DF_XRES;
int Settings::yRes = DF_YRES;

void Settings::RevertSettings()
{
	mouseInvert = DF_MOUSEINVERT;
	bloodEnabled = DF_BLOOD;
	fullScreen = DF_FULLSCREEN;
	vSync = DF_VSYNC;
	mouseSensitivity = DF_MOUSESENS;
	musicVolume = DF_MUSVOL;
	soundVolume = DF_SNDVOL;
	keyBackward = DF_KBACK;
	keyForward = DF_KFOR;
	keyLeft = DF_KLEF;
	keyRight = DF_KRIG;
	keyJump = DF_KJUMP;
	keyRevive = DF_KREV;
	keySlide = DF_KSLIDE;
	xRes = DF_XRES;
	yRes = DF_YRES;
	fastGraphics = DF_FASTGRAPHICS;
	difficulty = DF_DIFF;
}

void Settings::LoadSettings(Context* context)
{
	FileSystem* fileSystem = context->GetSubsystem<FileSystem>();
	ResourceCache* cache = context->GetSubsystem<ResourceCache>();
	File* file = new File(context);
	bool succ = file->Open(fileSystem->GetProgramDir() + GAMESETTINGS_PATH, FILE_READ);
	if (succ && file->GetSize() > 1)
	{
		fastGraphics = file->ReadBool();
		mouseInvert = file->ReadBool();
		bloodEnabled = file->ReadBool();
		fullScreen = file->ReadBool();
		vSync = file->ReadBool();

		mouseSensitivity = file->ReadFloat();
		musicVolume = file->ReadFloat();
		soundVolume = file->ReadFloat();

		keyBackward = file->ReadInt();
		keyForward = file->ReadInt();
		keyLeft = file->ReadInt();
		keyRight = file->ReadInt();
		keyJump = file->ReadInt();
		keyRevive = file->ReadInt();
		keySlide = file->ReadInt();

		xRes = file->ReadInt();
		yRes = file->ReadInt();

		difficulty = file->ReadFloat();
		
		file->Close();
	}
	else
	{
		file->Close();
		SaveSettings(context);
	}
}

void Settings::SaveSettings(Context* context)
{
	FileSystem* fileSystem = context->GetSubsystem<FileSystem>();
	ResourceCache* cache = context->GetSubsystem<ResourceCache>();
	File* file = new File(context);
	bool succ = file->Open(fileSystem->GetProgramDir() + GAMESETTINGS_PATH, FILE_WRITE);
	if (succ)
	{
		file->WriteBool(fastGraphics);
		file->WriteBool(mouseInvert);
		file->WriteBool(bloodEnabled);
		file->WriteBool(fullScreen);
		file->WriteBool(vSync);

		file->WriteFloat(mouseSensitivity);
		file->WriteFloat(musicVolume);
		file->WriteFloat(soundVolume);

		file->WriteInt(keyBackward);
		file->WriteInt(keyForward);
		file->WriteInt(keyLeft);
		file->WriteInt(keyRight);
		file->WriteInt(keyJump);
		file->WriteInt(keyRevive);
		file->WriteInt(keySlide);

		file->WriteInt(xRes);
		file->WriteInt(yRes);

		file->WriteFloat(difficulty);
		file->Close();
	}
}

bool Settings::IsKeyDown(Input* input, int key) //A hack that subtitutes certain unused key codes for mouse buttons for simplicity's sake.
{
	if (key == KEY_SCROLLLOCK && input->GetMouseButtonDown(MOUSEB_LEFT))
		return true;
	else if (key == KEY_RGUI && input->GetMouseButtonDown(MOUSEB_RIGHT))
		return true;
	else if (key == KEY_PAUSE && input->GetMouseButtonDown(MOUSEB_MIDDLE))
		return true;
	else
		return input->GetKeyDown(key);
}

float Settings::ScaleWithDifficulty(const float easyValue, const float hardValue, const float unholyValue)
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