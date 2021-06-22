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
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>

using namespace Urho3D;

class WeakChild : public LogicComponent //This class is for nodes created for other nodes, that should be deleted alongside them, but shouldn't have to inherit position.
{
	URHO3D_OBJECT(WeakChild, LogicComponent);
public:
	WeakChild(Context* context);
	static void RegisterObject(Context* context);
	static void MakeWeakChild(Node* child, Node* parent, bool autoPos = false, bool autoRot = false);
	virtual void FixedUpdate(float timeStep);
	~WeakChild();
	WeakPtr<Node> parent;
protected:
	bool matchPosition;
	bool matchRotation;
};

