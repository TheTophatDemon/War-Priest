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
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>

using namespace Urho3D;

class Gameplay;
class Projectile : public LogicComponent
{
public:
	Projectile(Context* context);
	virtual void Start() override;
	~Projectile();

	WeakPtr<Node> owner;

	static StringHash E_PROJECTILEHIT;
	static StringHash P_PERPETRATOR;
	static StringHash P_VICTIM;
	static StringHash P_DAMAGE;
protected:
	virtual void Die() = 0;
	void PreUpdate(float timeStep);
	void PostUpdate(float timeStep);
	void DoDamage(Node* victim, const int damage);
	void ForceFieldResponse(Node* otherNode, const float turnSpeed);

	SharedPtr<Gameplay> game;
	SharedPtr<ResourceCache> cache;
	SharedPtr<PhysicsWorld> physworld;
	SharedPtr<Scene> scene;
	SharedPtr<RigidBody> body;

	float lifeTimer;
	float deltaTime;
	bool hit;
	//The node shouldn't be removed in the middle of an update; use this instead.
	bool killMe;
};

