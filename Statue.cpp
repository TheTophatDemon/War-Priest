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
#include "ChaosCaliph.h"
#include "Settings.h"

Statue::Statue(Context* context) : 
	LogicComponent(context),
	health(100),
	shakeTimer(0.0f),
	radius(6.0f),
	deathTimer(0.0f)
{
}

Statue* Statue::MakeStatueComponent(Context* context, const int hp)
{
	Statue* statue = new Statue(context);
	statue->health = hp;
	return statue;
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
	translatePosition = originalPosition;
	radius *= (node_->GetScale().x_ / 1.75f); //As thus, the radius increases with scale

	soundSource = node_->CreateComponent<SoundSounder>();

	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Statue, OnCollision));
	SubscribeToEvent(Projectile::E_PROJECTILEHIT, URHO3D_HANDLER(Statue, OnProjectileHit));
}

void Statue::Damage(const int amount, const bool silent)
{
	if (shakeTimer < 0.5f) 
	{
		health -= amount * Settings::ScaleWithDifficulty(3.0f, 1.0f, 0.75f);
		if (!silent)
		{
			soundSource->Play("Sounds/env_damage.wav", true);
			shakeTimer = 1.0f;
		}
		if (health <= 0.0f)
		{
			soundSource->Play("Sounds/env_destruction.wav", true);
		}
	}
}

void Statue::FixedUpdate(float timeStep)
{
	if (shakeTimer >= 0.0f)
	{
		if (health > 0) shakeTimer -= timeStep; //It'll keep shaking forever if it's out of health
		if (shakeTimer < 0.0f)
		{
			node_->SetWorldPosition(translatePosition);
		}
		else 
		{
			const float rand = Random(-0.2f * shakeTimer, 0.2f * shakeTimer);
			node_->SetWorldPosition(translatePosition + Vector3(rand, -rand, -rand * 2.0f));
		}
		int smokeChance = 8;
		if (health <= 0) smokeChance = 2;
		if (Random(0, smokeChance) == 1)
		{
			Zeus::PuffOfSmoke(scene, originalPosition
				+ Vector3(Random(-radius,radius), Random(-1.0f,1.0f), Random(-radius,radius)), Random(0.5f, 1.5f));
		}
	}
	if (health <= 0)
	{
		deathTimer += timeStep;
		translatePosition += Vector3(0.0f, -timeStep * 6.0f, 0.0f);
		if (deathTimer > radius / 2.0f)
		{
			node_->Remove();
		}
	}
}

void Statue::OnCollision(StringHash eventType, VariantMap& eventData)
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
	Node* perpetrator = (Node*)eventData[Projectile::P_PERPETRATOR].GetPtr();
	Node* victim = (Node*)eventData[Projectile::P_VICTIM].GetPtr();
	const int damage = eventData[Projectile::P_DAMAGE].GetInt();
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
