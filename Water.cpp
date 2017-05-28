#include "Water.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <iostream>

#include "TempEffect.h"

Water::Water(Context* context) : LogicComponent(context)
{
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

	if (node_->HasComponent<RigidBody>()) 
	{
		body = node_->GetComponent<RigidBody>();
	}
	else 
	{
		body = node_->CreateComponent<RigidBody>();
		body->SetTrigger(true);
	}

	if (node_->HasComponent<CollisionShape>())
	{
		shape = node_->GetComponent<CollisionShape>();
	}
	else
	{
		shape = node_->CreateComponent<CollisionShape>();
		shape->SetBox(Vector3(4.0f, 0.001f, 4.0f), Vector3::ZERO, Quaternion::IDENTITY);
	}

	SubscribeToEvent(GetNode(), E_NODECOLLISIONSTART, URHO3D_HANDLER(Water, OnCollisionEnter));
}

void Water::FixedUpdate(float timeStep)
{
	
}

void Water::OnCollisionEnter(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (other->GetName() != "player")
	{
		TempEffect* te = new TempEffect(context_);
		te->life = 1.0f;
		other->AddComponent(te, 413, LOCAL);

		ParticleEmitter* prt = other->CreateComponent<ParticleEmitter>();
		prt->SetEffect(cache->GetResource<ParticleEffect>("Particles/splash.xml"));
		
		SoundSource3D* s = other->CreateComponent<SoundSource3D>();
		s->SetSoundType("ENVIRONMENT");
		s->Play(cache->GetResource<Sound>("Sounds/env_splash.wav"));
	}
}

Water::~Water()
{
}
