#include "Settings.h"
#include <iostream>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/File.h>
#include "SettingsMenu.h"

String Settings::GAMESETTINGS_PATH = "/Data/gamesettings.bin";
StringHash Settings::E_SETTINGSCHANGED = StringHash("SettingsChanged");

#define DF_FASTGRAPHICS false
#define DF_MOUSEINVERT false
#define DF_BLOOD true
#define DF_FULLSCREEN false
#define DF_VSYNC false

#define DF_MOUSESENS 0.25f
#define DF_MUSVOL 0.5f
#define DF_SNDVOL 1.0f
#define DF_DIFF 1.0f

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

const float Settings::UNHOLY_THRESHOLD = 1.4f;

SharedPtr<UInput> Settings::keyBackward;
SharedPtr<UInput> Settings::keyForward;
SharedPtr<UInput> Settings::keyLeft;
SharedPtr<UInput> Settings::keyRight;
SharedPtr<UInput> Settings::keyJump;
SharedPtr<UInput> Settings::keyRevive;
SharedPtr<UInput> Settings::keySlide;
SharedPtr<UInput>* Settings::inputs[] = { &keyForward, &keyBackward, &keyLeft, &keyRight, &keyJump, &keyRevive, &keySlide };

const int Settings::RES_X[] = { 1920, 1280, 800, 800, 640, 640 };
const int Settings::RES_Y[] = { 1080, 720, 600, 450, 480, 360 };

int Settings::xRes = DF_XRES;
int Settings::yRes = DF_YRES;

UInput::UInput() { name = "ERROR"; input = nullptr; }
UInput::UInput(String name, Input* input)
{
	this->name = name;
	this->input = input;
	this->verboseName = name;
}

KeyInput::KeyInput(const int keyCode, Input* input) : UInput(input->GetKeyName(keyCode), input), keyCode(keyCode) {}
bool KeyInput::isDown() { return input->GetKeyDown(keyCode); }
bool KeyInput::isPressed() { return input->GetKeyPress(keyCode); }

MouseInput::MouseInput(const int button, Input* input) : UInput("", input), button(button) 
{
	switch (button)
	{
	case MOUSEB_LEFT:
		name = "LMB";
		verboseName = "the Left Mouse Button";
		break;
	case MOUSEB_RIGHT:
		name = "RMB";
		verboseName = "the Right Mouse Button";
		break;
	case MOUSEB_MIDDLE:
		name = "MMB";
		verboseName = "the Middle Mouse Button";
		break;
	}
}
bool MouseInput::isDown() { return input->GetMouseButtonDown(button); }
bool MouseInput::isPressed() { return input->GetMouseButtonPress(button); }

void Settings::RevertSettings(Context* context)
{
	Input* input = context->GetSubsystem<Input>();
	mouseInvert = DF_MOUSEINVERT;
	bloodEnabled = DF_BLOOD;
	fullScreen = DF_FULLSCREEN;
	vSync = DF_VSYNC;
	mouseSensitivity = DF_MOUSESENS;
	musicVolume = DF_MUSVOL;
	soundVolume = DF_SNDVOL;
	keyBackward = SharedPtr<UInput>(new KeyInput(KEY_S, input));
	keyForward = SharedPtr<UInput>(new KeyInput(KEY_W, input));
	keyLeft = SharedPtr<UInput>(new KeyInput(KEY_A, input));
	keyRight = SharedPtr<UInput>(new KeyInput(KEY_D, input));
	keyJump = SharedPtr<UInput>(new KeyInput(KEY_SPACE, input));
	keyRevive = SharedPtr<UInput>(new MouseInput(MOUSEB_LEFT, input));
	keySlide = SharedPtr<UInput>(new MouseInput(MOUSEB_RIGHT, input));
	xRes = DF_XRES;
	yRes = DF_YRES;
	fastGraphics = DF_FASTGRAPHICS;
	difficulty = DF_DIFF;
}

void Settings::LoadSettings(Context* context)
{
	Input* input = context->GetSubsystem<Input>();
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

		for (int i = 0; i < NUM_INPUTS; ++i)
		{
			unsigned char type = file->ReadUByte();
			switch (type)
			{
			case 0: //KEY
				*inputs[i] = SharedPtr<UInput>(new KeyInput(file->ReadInt(), input));
				break;
			case 1: //MOUSE
				*inputs[i] = SharedPtr<UInput>(new MouseInput(file->ReadInt(), input));
				break;
			}
		}

		xRes = file->ReadInt();
		yRes = file->ReadInt();

		difficulty = file->ReadFloat();
		
		file->Close();
	}
	else
	{
		std::cout << "SETTINGS FILE NOT FOUND. IT HAS BEEN CREATED." << std::endl;
		file->Close();
		RevertSettings(context);
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

		for (int i = 0; i < NUM_INPUTS; ++i)
		{
			KeyInput* k = dynamic_cast<KeyInput*>(inputs[i]->Get());
			if (k)
			{
				file->WriteUByte(0U);
				file->WriteInt(k->keyCode);
			}
			else
			{
				MouseInput* m = dynamic_cast<MouseInput*>(inputs[i]->Get());
				if (m)
				{
					file->WriteUByte(1U);
					file->WriteInt(m->button);
				}
			}
		}

		file->WriteInt(xRes);
		file->WriteInt(yRes);

		file->WriteFloat(difficulty);
		file->Close();
	}
}