#include "Debris.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <iostream>

Debris::Debris(Context* context) : LogicComponent(context)
{
	damage = 15;
}

void Debris::RegisterObject(Context* context)
{
	context->RegisterFactory<Debris>();
}

void Debris::Start()
{
	scene = GetScene();
	body = node_->GetComponent<RigidBody>();
	physworld = scene->GetComponent<PhysicsWorld>();
	//Make glowy stuff here
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Debris, OnCollision));
}

void Debris::FixedUpdate(float timeStep)
{
	if (body->GetLinearVelocity().Length() < 2.0f)
	{
		node_->RemoveComponent<Debris>();
	}
}

void Debris::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (otherBody->GetCollisionLayer() & 128)
	{
		VariantMap map = VariantMap();
		map.Insert(Pair<StringHash, Variant>(StringHash("perpetrator"), node_));
		map.Insert(Pair<StringHash, Variant>(StringHash("victim"), other));
		map.Insert(Pair<StringHash, Variant>(StringHash("damage"), damage));
		SendEvent(StringHash("ProjectileHit"), map);
		node_->RemoveComponent<Debris>();
	}
}

Debris::~Debris()
{
}
