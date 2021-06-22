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

#include "Debris.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Audio/Sound.h>
#include <iostream>
#include "../Zeus.h"
#include "../Gameplay.h"

Debris::Debris(Context* context) : Projectile(context),
	damage(15),
	dieTime(-1.0f),
	linearVelocity(0.0f)
{
}

void Debris::RegisterObject(Context* context)
{
	context->RegisterFactory<Debris>();
}

void Debris::Start()
{
	SetUpdateEventMask(USE_FIXEDUPDATE);
	
	Projectile::Start();

	crashSource = node_->GetOrCreateComponent<SoundSource3D>();
	crashSource->SetSoundType("GAMEPLAY");

	SubscribeToEvent(GetNode(), E_NODECOLLISIONSTART, URHO3D_HANDLER(Debris, OnCollisionStart));
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Debris, OnCollision));
}

void Debris::FixedUpdate(float timeStep)
{
	PreUpdate(timeStep);

	linearVelocity = body->GetLinearVelocity().Length();
	if (linearVelocity < 4.0f)
	{
		Die();
	}
	else if (linearVelocity < 10.0f)
	{
		//Don't show screen warnings for slow ones.
		node_->RemoveTag("trackable_projectile");
	}
	else
	{
		const Vector3 plyDiff = (game->playerNode->GetWorldPosition() - node_->GetWorldPosition());
		if (plyDiff.DotProduct(body->GetLinearVelocity()) < 0.0f)
		{
			//Remove screen warnings for debris flying away from the player
			node_->RemoveTag("trackable_projectile");
		}
		else if (!node_->HasTag("trackable_projectile"))
		{
			//Otherwise, keep tracking
			node_->AddTag("trackable_projectile");
		}
	}
	if (hit) 
	{
		if (smokeNode) 
		{ 
			smokeNode->SetWorldPosition(node_->GetWorldPosition()); 
		}
		if (lifeTimer - dieTime > 0.5f)
		{
			killMe = true;
		}
	}

	PostUpdate(timeStep);
}

void Debris::OnCollisionStart(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (other != owner) 
	{
		if (otherBody->GetCollisionLayer() & 4)
		{
			DoDamage(other, damage);
			Die();
		}
		else if (otherBody->GetCollisionLayer() & 256)
		{
			node_->RemoveTag("trackable_projectile");
		}
		if ((otherBody->GetCollisionLayer() & 198 || other->HasTag("debris")) && linearVelocity > 10.0f) //64+128+2+4
		{
			crashSource->Play(cache->GetResource<Sound>("Sounds/env_rock.wav"), 44100.0f + Random(-1500.0f, 1500.0f));
		}
	}
}

void Debris::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (other != owner)
	{
		//Handle forcefield interactions with more FORCE than usual
		if (otherBody->GetCollisionLayer() & 16) 
		{
			const Vector3 diff = other->GetWorldPosition() - node_->GetWorldPosition();
			float push = 0.0f;
			if (diff != Vector3::ZERO)
			{
				push = 8.0f + (12.0f / diff.LengthSquared());
			}
			if (other->HasTag("tempshield"))
			{
				otherBody->ApplyImpulse(diff.Normalized() * push * otherBody->GetMass());
			}
			else if (other->HasTag("blackhole"))
			{
				otherBody->ApplyImpulse(-diff.Normalized() * push * otherBody->GetMass());
			}
		}
	}
}

void Debris::Die()
{
	if (!hit) 
	{
		hit = true;
		node_->RemoveTag("trackable_projectile");
		smokeNode = Zeus::PuffOfSmoke(GetScene(), node_->GetWorldPosition(), 2.0f);
		dieTime = lifeTimer;
	}
}

Debris::~Debris()
{
}
