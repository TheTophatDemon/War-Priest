#include "Settings.h"
#include <iostream>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/File.h>

String Settings::GAMESETTINGS_PATH = "Data/gamesettings.bin";
String Settings::VIDEOSETTINGS_PATH = "Data/videosettings.xml";

#define DF_FASTGRAPHICS false
#define DF_MOUSEINVERT false
#define DF_BLOOD true
#define DF_FULLSCREEN false
#define DF_VSYNC false

#define DF_MOUSESENS 0.25f
#define DF_MUSVOL 0.5f
#define DF_SNDVOL 0.5f

#define DF_KBACK 115
#define DF_KFOR 119
#define DF_KLEF 97
#define DF_KRIG 100
#define DF_KJUMP 32
#define DF_KREV 1073741895
#define DF_KSLIDE 1073742055

bool Settings::fastGraphics = DF_FASTGRAPHICS;
bool Settings::mouseInvert = DF_MOUSEINVERT;
bool Settings::bloodEnabled = DF_BLOOD;
bool Settings::fullScreen = DF_FULLSCREEN;
bool Settings::vSync = DF_VSYNC;

float Settings::mouseSensitivity = DF_MOUSESENS;
float Settings::musicVolume = DF_MUSVOL;
float Settings::soundVolume = DF_SNDVOL;

int Settings::keyBackward = DF_KBACK;
int Settings::keyForward = DF_KFOR;
int Settings::keyLeft = DF_KLEF;
int Settings::keyRight = DF_KRIG;
int Settings::keyJump = DF_KJUMP;
int Settings::keyRevive = DF_KREV;
int Settings::keySlide = DF_KSLIDE;

void Settings::RevertSettings()
{
	fastGraphics = DF_FASTGRAPHICS;
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
}

void Settings::LoadSettings(Context* context)
{
	ResourceCache* cache = context->GetSubsystem<ResourceCache>();
	File* file = new File(context);
	bool succ = file->Open(GAMESETTINGS_PATH, FILE_READ);
	if (!succ)
	{
		std::cout << "YEAH BOY" << std::endl;
	}
	else
	{
		file->Close();
	}

	XMLFile* videoSettings = cache->GetResource<XMLFile>(VIDEOSETTINGS_PATH);
}

void Settings::SaveSettings(Context* context)
{
	
}