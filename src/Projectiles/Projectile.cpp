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

#include "Projectile.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <iostream>
#include <algorithm>

#include "../Actors/Enemy.h"
#include "../Actors/Player.h"
#include "../Gameplay.h"

StringHash Projectile::E_PROJECTILEHIT = StringHash("ProjectileHit");
StringHash Projectile::P_PERPETRATOR = StringHash("perpetrator");
StringHash Projectile::P_VICTIM = StringHash("victim");
StringHash Projectile::P_DAMAGE = StringHash("damage");

Projectile::Projectile(Context* context) : LogicComponent(context),
	lifeTimer(0.0f),
	deltaTime(0.0f),
	hit(false),
	killMe(false)
{
}

void Projectile::Start()
{
	game = GetScene()->GetComponent<Gameplay>();
	cache = GetSubsystem<ResourceCache>();
	scene = GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	body = node_->GetComponent<RigidBody>();
}

void Projectile::PreUpdate(float timeStep)
{
	lifeTimer += timeStep;
	deltaTime = timeStep;
}

void Projectile::PostUpdate(float timeStep)
{
	if (killMe)
	{
		node_->Remove();
	}
}

void Projectile::ForceFieldResponse(Node* otherNode, const float turnSpeed)
{
	Quaternion facingRotation;
	if (otherNode->HasTag("tempshield"))
	{
		facingRotation.FromLookRotation((node_->GetWorldPosition() - otherNode->GetWorldPosition()).Normalized(), Vector3::UP);
	}
	else if (otherNode->HasTag("blackhole"))
	{
		facingRotation.FromLookRotation((otherNode->GetWorldPosition() - node_->GetWorldPosition()).Normalized(), Vector3::UP);
	}
	node_->SetWorldRotation(node_->GetWorldRotation().Slerp(facingRotation, turnSpeed * deltaTime));
}

void Projectile::DoDamage(Node* victim, const int damage)
{
	VariantMap map = VariantMap();
	map.Insert(Pair<StringHash, Variant>(P_PERPETRATOR, node_));
	map.Insert(Pair<StringHash, Variant>(P_VICTIM, victim));
	map.Insert(Pair<StringHash, Variant>(P_DAMAGE, damage));
	SendEvent(E_PROJECTILEHIT, map);
}

Projectile::~Projectile()
{
}

/*
void Projectile::MoveSphereCast(const float timeStep, const Vector3 movement, const float speed, const float radius, const float length, const unsigned int collisionMask)
{
	PhysicsRaycastResult result;
	float movLength = movement.Length();
	if (movLength == 0.0f) movLength = 1.0f;
	physworld->SphereCast(result, Ray(node_->GetWorldPosition(), movement / movLength), radius, movLength + length, collisionMask);
	if (result.body_)
	{
		if (result.body_->GetNode() != owner)
		{
			OnHit(result);
		}
	}
	else 
	{
		physworld->RaycastSingle(result, Ray(node_->GetWorldPosition(), movement / movLength), movLength + length, collisionMask);
		if (result.body_)
		{
			if (result.body_->GetNode() != owner)
			{
				OnHit(result);
			}
		}
	}
	node_->Translate(movement, TS_WORLD);
}
*/