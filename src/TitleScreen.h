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

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/Button.h>
#include "Menu/Menu.h"


using namespace Urho3D;

class GunPriest;
class Gameplay;
class TitleScreen : public LogicComponent //Manages and displays non-game UI
{
	URHO3D_OBJECT(TitleScreen, LogicComponent);
public:
	TitleScreen(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	~TitleScreen();

	SharedPtr<UIElement> ourUI;
	SharedPtr<Gameplay> game;
	GunPriest* gunPriest;

	void MakeMenus();
	void SetMenu(GP::Menu* newMenu);

	SharedPtr<GP::Menu> titleMenu;
	SharedPtr<GP::Menu> levelSelectMenu;
	SharedPtr<GP::Menu> settingsMenu;

	SharedPtr<SoundSource> soundSource;
protected:
	void OnEvent(StringHash eventType, VariantMap& eventData);
	void OnUpdate(StringHash eventType, VariantMap& eventData);

	Engine* engine_;
	SharedPtr<Renderer> renderer;
	SharedPtr<UI> ui;
	SharedPtr<Input> input;
	SharedPtr<ResourceCache> cache;
	SharedPtr<Audio> audio;

	SharedPtr<GP::Menu> currentMenu;
	SharedPtr<Node> soundNode;
	SharedPtr<SoundSource> scrollSource;

	float time;
};

