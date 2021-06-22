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
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Graphics/Material.h>

using namespace Urho3D;

class Launchpad : public LogicComponent
{
	URHO3D_OBJECT(Launchpad, LogicComponent);
public:
	Launchpad(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~Launchpad();

	float launchForce;
private:
	void OnCollision(StringHash eventType, VariantMap& eventData);
	
	SharedPtr<PhysicsWorld> physworld;
	SharedPtr<Scene> scene;
	SharedPtr<ResourceCache> cache;

	SharedPtr<SoundSource3D> soundSource;
	SharedPtr<Node> topNode;
	SharedPtr<Node> midNode;
	SharedPtr<Node> baseNode;

	SharedPtr<Material> material;
	SharedPtr<ValueAnimation> valAnim;

	float elapsedTime = 0.0f;
	float turnSpeedModifier = 0.0f;
};

