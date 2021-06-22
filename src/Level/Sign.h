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
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Input/Input.h>

using namespace Urho3D;

class Gameplay;
namespace GP 
{
	class Sign : public LogicComponent
	{
		URHO3D_OBJECT(Sign, LogicComponent);
	public:
		Sign(Context* context);
		static void RegisterObject(Context* context);
		virtual void Start() override;
		virtual void FixedUpdate(float timeStep) override;
		~Sign();
	protected:
		String ParseMessage(const String& message);
		WeakPtr<Gameplay> game;
		Input* input;
	};
}

