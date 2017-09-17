#include "Statue.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Audio/Audio.h>
#include <iostream>

#include "TempEffect.h"
#include "Projectile.h"
#include "Debris.h"
#include "Zeus.h"

Statue::Statue(Context* context) : 
	LogicComponent(context),
	health(100),
	shakeTimer(0.0f),
	radius(7.0f)
{
}

void Statue::RegisterObject(Context* context)
{
	context->RegisterFactory<Statue>();
}

void Statue::Start()
{
	scene = GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	cache = GetSubsystem<ResourceCache>();
	body = node_->GetComponent<RigidBody>();

	originalPosition = node_->GetWorldPosition();

	SubscribeToEvent(GetNode(), E_NODECOLLISIONSTART, URHO3D_HANDLER(Statue, OnCollisionEnter));
	SubscribeToEvent(Projectile::E_PROJECTILEHIT, URHO3D_HANDLER(Statue, OnProjectileHit));
}

void Statue::Damage(const int amount, const bool silent)
{
	health -= amount;
	if (!silent) shakeTimer = 1.0f;
}

void Statue::FixedUpdate(float timeStep)
{
	if (shakeTimer >= 0.0f)
	{
		shakeTimer -= timeStep;
		if (shakeTimer < 0.0f)
		{
			node_->SetWorldPosition(originalPosition);
		}
		else 
		{
			const float rand = Random(-0.1f, 0.1f);
			node_->SetWorldPosition(originalPosition + Vector3(rand, -rand, -rand * 2.0f));
		}
		if (Random(0, 10) == 1)
		{
			Zeus::PuffOfSmoke(scene, node_->GetWorldPosition()
				+ Vector3(Random(-radius,radius), Random(-1.0f,1.0f), Random(-radius,radius)), Random(0.5f, 1.5f));
		}
	}
}

void Statue::OnCollisionEnter(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (other->HasTag("debris"))
	{
		Debris* deb = other->GetComponent<Debris>();
		Damage(deb->damage);
	}
}

void Statue::OnProjectileHit(StringHash eventType, VariantMap& eventData)
{
	Node* perpetrator = (Node*)eventData["perpetrator"].GetPtr();
	Node* victim = (Node*)eventData["victim"].GetPtr();
	const int damage = eventData["damage"].GetInt();
	if (victim == GetNode())
	{
		Damage(damage);
	}
}

int Statue::GetHealth()
{
	return health;
}

Statue::~Statue()
{
}
