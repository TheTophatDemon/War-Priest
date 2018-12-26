#include "Debris.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <iostream>
#include "Zeus.h"
#include "Projectile.h"

Debris::Debris(Context* context) : LogicComponent(context)
{
	damage = 15;
	dieTimer = 0.0f;
	linearVelocity = 0.0f;
}

void Debris::RegisterObject(Context* context)
{
	context->RegisterFactory<Debris>();
}

void Debris::Start()
{
	SetUpdateEventMask(USE_FIXEDUPDATE);
	scene = GetScene();
	body = node_->GetComponent<RigidBody>();
	physworld = scene->GetComponent<PhysicsWorld>();
	cache = GetSubsystem<ResourceCache>();

	crashSource = node_->GetOrCreateComponent<SoundSource3D>();
	crashSource->SetSoundType("GAMEPLAY");

	SubscribeToEvent(GetNode(), E_NODECOLLISIONSTART, URHO3D_HANDLER(Debris, OnCollisionStart));
}

void Debris::FixedUpdate(float timeStep)
{
	linearVelocity = body->GetLinearVelocity().Length();
	if (linearVelocity < 4.0f)
	{
		Die();
	}
	if (dieTimer > 0.0f) 
	{
		if (smokeNode) 
		{ 
			smokeNode->SetWorldPosition(node_->GetWorldPosition() ); 
		}
		dieTimer -= timeStep;
		if (dieTimer <= 0.0f)
		{
			node_->Remove();
		}
	}
}

void Debris::OnCollisionStart(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (other->GetName() == "player")
	{
		VariantMap map = VariantMap();
		map.Insert(Pair<StringHash, Variant>(Projectile::P_PERPETRATOR, node_));
		map.Insert(Pair<StringHash, Variant>(Projectile::P_VICTIM, other));
		map.Insert(Pair<StringHash, Variant>(Projectile::P_DAMAGE, damage));
		SendEvent(Projectile::E_PROJECTILEHIT, map);
		Die();
	}
	else if (otherBody->GetCollisionLayer() & 256) 
	{
		node_->RemoveTag("projectile");
	}

	if ( (otherBody->GetCollisionLayer() & 198 || other->HasTag("debris")) && linearVelocity > 10.0f) //64+128+2+4
	{
		crashSource->Play(cache->GetResource<Sound>("Sounds/env_rock.wav"), 44100.0f + Random(-1500.0f, 1500.0f));
	}
}

void Debris::Die()
{
	if (dieTimer == 0.0f) 
	{
		node_->RemoveTag("projectile");
		smokeNode = Zeus::PuffOfSmoke(GetScene(), node_->GetWorldPosition(), 2.0f);
		dieTimer = 0.5f;
	}
}

Debris::~Debris()
{
}
