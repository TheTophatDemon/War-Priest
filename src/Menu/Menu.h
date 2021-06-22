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
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/Button.h>
#include "../Gameplay.h"

class TitleScreen;
namespace GP 
{
	class Menu : public Object
	{
		URHO3D_OBJECT(Menu, Object);
	public:
		Menu(Context* context, TitleScreen* ts, SharedPtr<Gameplay> gm);
		virtual void OnEnter();
		virtual void OnLeave();
		virtual void Update(float timeStep);
		virtual ~Menu();
		String layoutPath;
	protected:
		SharedPtr<TitleScreen> titleScreen;
		SharedPtr<Gameplay> gameplay;
		SharedPtr<ResourceCache> cache;
		SharedPtr<UI> ui;
		void DisableTexts();
	};
}

