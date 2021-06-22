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
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/Audio/Audio.h>

using namespace Urho3D;

class Zeus //He does all the special effects
{
public:
	Zeus();
	static void RegisterZeus(Context* context);
	static Node* MakeLightBeam(Scene* scene, Vector3 position, const int sound = 1);
	static Node* MakeShield(Scene* scene, Vector3 position, float radius);
	static Node* PuffOfSmoke(Scene* scene, Vector3 position, float length);
	static Node* MakeExplosion(Scene* scene, Vector3 position, const float life, const float size = 1.0f);
	static void ApplyRadialDamage(Scene* scene, Node* perpetrator, const float radius, const int damage, const int mask);
	~Zeus();
};

