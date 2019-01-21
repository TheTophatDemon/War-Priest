#include "Water.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Resource/XMLFile.h>
#include <iostream>

#include "Actor.h"
#include "TempEffect.h"
#include "Zeus.h"
#include "Gameplay.h"

Water::Water(Context* context) : LogicComponent(context)
{
	elapsed = 0.0f;
	bobFactor = 1.0f;
}

void Water::RegisterObject(Context* context)
{
	context->RegisterFactory<Water>();
}

void Water::Start()
{
	scene = GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	cache = GetSubsystem<ResourceCache>();
	audio = GetSubsystem<Audio>();
	splashSound = cache->GetResource<Sound>("Sounds/env_splash.wav");

	bobFactor = node_->GetVar("bobFactor").GetFloat();
	if (bobFactor == 0.0f) bobFactor = 1.0f;

	if (node_->HasComponent<RigidBody>()) 
	{
		body = node_->GetComponent<RigidBody>();
	}
	else 
	{
		body = node_->CreateComponent<RigidBody>();
		body->SetTrigger(true);
		body->SetCollisionLayer(1 | 256);
	}

	if (node_->HasComponent<CollisionShape>())
	{
		shape = node_->GetComponent<CollisionShape>();
	}
	else
	{
		shape = node_->CreateComponent<CollisionShape>();
		float size = 8.0f;
		
		if (node_->HasComponent<StaticModel>())
		{
			StaticModel* s = node_->GetComponent<StaticModel>();
			size = s->GetBoundingBox().Size().x_;
		}
		shape->SetBox(Vector3(size, 0.001f, size), Vector3::ZERO, Quaternion::IDENTITY);
	}

	SubscribeToEvent(GetNode(), E_NODECOLLISIONSTART, URHO3D_HANDLER(Water, OnCollisionEnter));
}

void Water::FixedUpdate(float timeStep)
{
	elapsed += timeStep;
	node_->Translate(Vector3(0.0f, sinf(elapsed) * 0.005f * bobFactor, 0.0f), TS_LOCAL);
}

void Water::OnCollisionEnter(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (!other->HasTag("nodrowning")) 
	{
		if (!other->HasComponent<Actor>() && otherBody->GetMass() > 0)
		{
			TempEffect* te = new TempEffect(context_);
			te->life = 3.5f;
			other->AddComponent(te, 413, LOCAL);

			ParticleEmitter* prt = other->CreateComponent<ParticleEmitter>();
			prt->SetEffect(cache->GetResource<ParticleEffect>("Particles/splash.xml"));

			SoundSource3D* s = other->CreateComponent<SoundSource3D>();
			s->SetSoundType("GAMEPLAY");
			s->Play(splashSound);
		}
		/*else if (other->GetName() != "player") //God'll get the rest of em
		{
			Zeus::MakeLightBeam(scene, other->GetWorldPosition(), 2048.0f);
			other->Remove();
		}*/
	}
}

Water::~Water()
{
}
