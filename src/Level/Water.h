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
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>

using namespace Urho3D;

class Water : public LogicComponent
{
	URHO3D_OBJECT(Water, LogicComponent);
public:
	Water(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~Water();
protected:
	void OnCollisionEnter(StringHash eventType, VariantMap& eventData);
	SharedPtr<Scene> scene;
	SharedPtr<PhysicsWorld> physworld;
	SharedPtr<ResourceCache> cache;
	SharedPtr<RigidBody> body;
	SharedPtr<CollisionShape> shape;
	SharedPtr<Audio> audio;
	SharedPtr<Sound> splashSound;
	float elapsed;
	float bobFactor;
};

