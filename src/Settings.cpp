#include "Settings.h"
#include <iostream>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/File.h>
#include "SettingsMenu.h"

const String Settings::GAMESETTINGS_PATH = "/Data/gamesettings.bin";
const StringHash Settings::E_SETTINGSCHANGED = StringHash("SettingsChanged");

int Settings::xRes = 0;
int Settings::yRes = 0;
float Settings::mouseSensitivity = 0.0f;
float Settings::musicVolume = 0.0f;
float Settings::soundVolume = 0.0f;
float Settings::difficulty = 0.0f;
bool Settings::mouseInvert = false;
bool Settings::bloodEnabled = false;
bool Settings::fullScreen = false;
bool Settings::vSync = false;
bool Settings::fastGraphics = false;

const int Settings::RES_X[] = { 1920, 1280, 800, 800, 640, 640 };
const int Settings::RES_Y[] = { 1080, 720, 600, 450, 480, 360 };

const float Settings::UNHOLY_THRESHOLD = 1.4f;

Action::Action(String name) : name(name) {}

float Action::getValue()
{
	if (joyBinding.NotNull())
	{
		const float val = joyBinding->getValue();
		if (fabs(val) > Settings::DEADZONE) 
			return val;
	}
	if (binding.NotNull())
	{
		return binding->getValue();
	}
	return 0.0f;
}

bool Action::valueChanged()
{
	if (joyBinding.NotNull())
	{
		if (joyBinding->valueChanged()) return true;
	}
	if (binding.NotNull())
	{
		if (binding->valueChanged()) return true;
	}
	return false;
}

Action Settings::actions[] = {
	Action("Move Forward"),
	Action("Move Backward"),
	Action("Move Left"),
	Action("Move Right"),
	Action("Jump"),
	Action("Revive"),
	Action("Slide"),
	Action("Turn Camera Right"),
	Action("Turn Camera Left"),
	Action("Turn Camera Up"),
	Action("Turn Camera Down"),
	Action("Open Menu")
};

Binding::Binding() { name = "ERROR"; input = nullptr; }
Binding::Binding(String name, Input* input, BindingType type)
{
	this->name = name;
	this->input = input;
	this->verboseName = name;
	this->type = type;
}

//KEY BINDING
KeyBinding::KeyBinding(const int keyCode, Input* input) : Binding(input->GetKeyName(keyCode), input, BindingType::KEY), keyCode(keyCode) {}
float KeyBinding::getValue() { return input->GetKeyDown(keyCode) ? 1.0f : 0.0f; }
bool KeyBinding::valueChanged() { return input->GetKeyPress(keyCode); }

//MOUSE BINDING
MouseBinding::MouseBinding(const int button, Input* input) 
	: Binding("", input, BindingType::MOUSE), 
	button(button) 
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
float MouseBinding::getValue() { return input->GetMouseButtonDown(button) ? 1.0f : 0.0f; }
bool MouseBinding::valueChanged() { return input->GetMouseButtonPress(button); }

//JOYSTICK BUTTON BINDING
JoyButtBinding::JoyButtBinding(const int joyIndex, const int button, Input* input)
	: Binding("Joystick " + String(joyIndex) + " Button " + String(button), input, BindingType::JOYBUTT),
	joyIndex(joyIndex),
	button(button) {}
float JoyButtBinding::getValue() 
{ 
	if (input->GetNumJoysticks() <= joyIndex) return 0.0f; //Cancel if no joystick
	return input->GetJoystickByIndex(joyIndex)->GetButtonDown(button) ? 1.0f : 0.0f; 
};
bool JoyButtBinding::valueChanged() 
{ 
	if (input->GetNumJoysticks() <= joyIndex) return false; //Cancel if no joystick
	return input->GetJoystickByIndex(joyIndex)->GetButtonPress(button); 
};

//JOYSTICK AXIS BINDING
JoyAxisBinding::JoyAxisBinding(const int joyIndex, const int axis, const int sign, Input* input)
	: Binding("Joystick " + String(joyIndex) + " Axis " + String(axis) + (sign > 0 ? " (+)" : " (-)"), input, BindingType::JOYAXIS),
	joyIndex(joyIndex),
	axis(axis),
	sign(sign) {}
float JoyAxisBinding::getValue() 
{ 
	if (input->GetNumJoysticks() <= joyIndex) return 0.0f; //Cancel if no joystick
	return Max(0.0f, input->GetJoystickByIndex(joyIndex)->GetAxisPosition(axis) * (float)sign);
};
//The value is always changing; we can't keep track of that here.
bool JoyAxisBinding::valueChanged()
{
	return false;
};

//JOYSTICK HAT BINDING
JoyHatBinding::JoyHatBinding(const int joyIndex, const int hat, const int direction, Input* input)
	: Binding("Joystick " + String(joyIndex) + " Hat " + String(hat) + " Direction " + String(direction), input, BindingType::JOYHAT),
	joyIndex(joyIndex),
	hat(hat),
	direction(direction) {}
float JoyHatBinding::getValue()
{
	if (input->GetNumJoysticks() <= joyIndex) return 0.0f;
	return input->GetJoystickByIndex(joyIndex)->GetHatPosition(hat) == direction;
}
bool JoyHatBinding::valueChanged()
{
	return false;
}

void Settings::RevertSettings(Context* context)
{
	Input* input = context->GetSubsystem<Input>();

	mouseInvert = false;
	bloodEnabled = true;
	fullScreen = false;
	vSync = false;
	mouseSensitivity = 0.25f;
	musicVolume = 0.4f;
	soundVolume = 1.0f;
	xRes = 1280;
	yRes = 720;
	fastGraphics = false;
	difficulty = 1.0f;

	GetAction(ActionType::BACK).binding = new KeyBinding(KEY_S, input);
	GetAction(ActionType::FORWARD).binding = new KeyBinding(KEY_W, input);
	GetAction(ActionType::LEFT).binding = new KeyBinding(KEY_A, input);
	GetAction(ActionType::RIGHT).binding = new KeyBinding(KEY_D, input);
	GetAction(ActionType::JUMP).binding = new KeyBinding(KEY_SPACE, input);
	GetAction(ActionType::REVIVE).binding = new MouseBinding(MOUSEB_LEFT, input);
	GetAction(ActionType::SLIDE).binding = new MouseBinding(MOUSEB_RIGHT, input);
	GetAction(ActionType::CAM_RIGHT).binding = new KeyBinding(KEY_RIGHT, input);
	GetAction(ActionType::CAM_LEFT).binding = new KeyBinding(KEY_LEFT, input);
	GetAction(ActionType::CAM_UP).binding = new KeyBinding(KEY_UP, input);
	GetAction(ActionType::CAM_DOWN).binding = new KeyBinding(KEY_DOWN, input);
	GetAction(ActionType::MENU).binding = new KeyBinding(KEY_ESCAPE, input);
	for (int i = 0; i < static_cast<int>(ActionType::COUNT); ++i)
		actions[i].joyBinding = nullptr;
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

		for (int i = 0; i < static_cast<int>(ActionType::COUNT); ++i)
		{
			//Read Keyboard/Mouse Binding
			BindingType type = static_cast<BindingType>(file->ReadUByte());
			switch (type)
			{
				case BindingType::KEY: //KEY
					actions[i].binding = new KeyBinding(file->ReadInt(), input);
					break;
				case BindingType::MOUSE: //MOUSE
					actions[i].binding = new MouseBinding(file->ReadInt(), input);
					break;
				case BindingType::NONE:
					actions[i].binding = nullptr;
					break;
			}
			//Read Joystick Binding
			type = static_cast<BindingType>(file->ReadUByte());
			switch (type)
			{
				case BindingType::JOYBUTT: //JOY BUTTON
				{
					const int index = file->ReadInt();
					const int button = file->ReadInt();
					actions[i].joyBinding = new JoyButtBinding(index, button, input);
					break;
				}
				case BindingType::JOYAXIS:
				{
					const int index = file->ReadInt();
					const int axis = file->ReadInt();
					const bool positive = file->ReadBool();
					actions[i].joyBinding = new JoyAxisBinding(index, axis, positive ? 1 : -1, input);
					break;
				}
				case BindingType::NONE:
					actions[i].joyBinding = nullptr;
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

		for (int i = 0; i < static_cast<int>(ActionType::COUNT); ++i)
		{
			if (actions[i].binding.NotNull())
			{
				file->WriteUByte(static_cast<unsigned char>(actions[i].binding->type));
				switch (actions[i].binding->type)
				{
					case BindingType::KEY:
					{
						KeyBinding* key = dynamic_cast<KeyBinding*>(actions[i].binding.Get());
						file->WriteInt(key->keyCode);
						break;
					}
					case BindingType::MOUSE:
					{
						MouseBinding* mouse = dynamic_cast<MouseBinding*>(actions[i].binding.Get());
						file->WriteInt(mouse->button);
						break;
					}
				}
			}
			else
			{
				file->WriteUByte(static_cast<unsigned char>(BindingType::NONE));
			}
			if (actions[i].joyBinding.NotNull())
			{
				file->WriteUByte(static_cast<unsigned char>(actions[i].joyBinding->type));
				switch (actions[i].joyBinding->type)
				{
					case BindingType::JOYBUTT:
					{
						JoyButtBinding* joyButt = dynamic_cast<JoyButtBinding*>(actions[i].joyBinding.Get());
						file->WriteInt(joyButt->joyIndex);
						file->WriteInt(joyButt->button);
						break;
					}
					case BindingType::JOYAXIS:
					{
						JoyAxisBinding* joy = dynamic_cast<JoyAxisBinding*>(actions[i].joyBinding.Get());
						file->WriteInt(joy->joyIndex);
						file->WriteInt(joy->axis);
						file->WriteBool(joy->sign > 0);
						break;
					}
				}
			}
			else
			{
				file->WriteUByte(static_cast<unsigned char>(BindingType::NONE));
			}
		}

		file->WriteInt(xRes);
		file->WriteInt(yRes);

		file->WriteFloat(difficulty);
		file->Close();
	}
}