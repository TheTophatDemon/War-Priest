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

#include "Missile.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <iostream>
#include "../Settings.h"
#include "../TempEffect.h"
#include "../Zeus.h"

Missile::Missile(Context* context) : Projectile(context),
	state(0),
	targetOffset(0.0f,0.0f,0.0f),
	speed(1.0f),
	orgSpeed(1.0f)
{
}

void Missile::Start()
{
	SetUpdateEventMask(USE_FIXEDUPDATE);

	Projectile::Start();

	emitterNode = node_->GetChild("smoke");
	emitter = emitterNode->GetComponent<ParticleEmitter>();
	emitter->SetEmitting(true);

	orgSpeed = speed;

	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Missile, OnCollision));
}

void Missile::RegisterObject(Context* context)
{
	context->RegisterFactory<Missile>();
}

void Missile::FixedUpdate(float timeStep)
{
	PreUpdate(timeStep);
	
	if (lifeTimer > 25.0f)
	{
		Die();
	}

	switch (state)
	{
	case 0:
		speed -= (timeStep * 35.0f) * Sign(orgSpeed);
		if (speed <= 0.0f)
		{
			targetOffset = Vector3(Random(-1.0f, 1.0f), 2.0f, Random(-1.0f, 1.0f));
			state++;
		}
		break;
	case 1:
	{
		if (target.Get() != nullptr)
		{
			const Vector3 diff = target->GetWorldPosition() + targetOffset - node_->GetWorldPosition();
			Quaternion dest = Quaternion();
			dest.FromLookRotation(diff.Normalized(), Vector3::UP);
			node_->SetWorldRotation(node_->GetWorldRotation().Slerp(dest, 5.0f * timeStep));
			speed += (timeStep * 20.5f) * Sign(orgSpeed);
			if (fabs(speed) > fabs(orgSpeed))
			{
				speed = orgSpeed;
			}
			if (diff.Length() < 20.0f && fabs(speed) >= fabs(orgSpeed) / 2.0f)
			{
				state++;
			}
		}
		break;
	}
	case 2:
		//Just keep going until we explode
		break;
	case 3: //For when it's deflected by a shield
		speed += (timeStep * 30.0f) * Sign(orgSpeed);
		if (fabs(speed - orgSpeed) <= 0.0f) speed = orgSpeed;
		break;
	}
	node_->Translate(node_->GetWorldRotation() * (Vector3::FORWARD * speed * timeStep), TS_WORLD);

	PostUpdate(timeStep);
}

void Missile::Die()
{
	emitter->SetEmitting(false);

	Node* explosion = Zeus::MakeExplosion(scene, node_->GetWorldPosition(), 2.0f, 3.5f);
	Zeus::ApplyRadialDamage(scene, node_, 4.5f, Settings::ScaleWithDifficulty(7.0f, 10.0f, 13.0f), 132); //128 + 4

	SoundSource3D* s = explosion->CreateComponent<SoundSource3D>();
	s->SetSoundType("GAMEPLAY");
	s->Play(cache->GetResource<Sound>("Sounds/env_explode.wav"));

	killMe = true;
	
	emitterNode->SetParent(scene);
	TempEffect* t = new TempEffect(context_);
	t->life = 2.0f;
	emitterNode->AddComponent(t, 1212, LOCAL);
}

void Missile::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();

	if (other != owner || state > 0) //Will collide with owner, but not while it's still being launched
	{
		if (otherBody->GetCollisionLayer() & 198 || other == owner) //2 + 128 + 64 + 4
		{
			Die();
		}
		else if (otherBody->GetCollisionLayer() & 16)
		{
			ForceFieldResponse(other, 2.0f);
			state = 3;
		}
	}
}

Node* Missile::MakeMissile(Scene* sc, const Vector3 position, const Quaternion rotation, Node* owner, Node* target)
{
	Missile* m = new Missile(sc->GetContext());
	m->owner = owner;
	m->speed = 55.0f + Settings::ScaleWithDifficulty(-20.0f, 0.0f, 20.0f);
	m->target = WeakPtr<Node>(target);

	Node* n = sc->CreateChild();
	n->LoadXML(sc->GetSubsystem<ResourceCache>()->GetResource<XMLFile>("Objects/projectile_missile.xml")->GetRoot());
	n->SetPosition(position);
	n->SetRotation(Quaternion(90.0f, Vector3::UP) * rotation);
	n->AddComponent(m, 333, LOCAL);

	return n;
}

Missile::~Missile()
{
}
