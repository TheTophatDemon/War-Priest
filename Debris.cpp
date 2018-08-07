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
	//Make glowy stuff here
	StaticModel* sm = node_->GetComponent<StaticModel>();
	if (sm)
	{
		glowNode = node_->CreateChild();
		StaticModel* gm = glowNode->CreateComponent<StaticModel>();
		gm->SetModel(sm->GetModel());
		gm->SetMaterial(cache->GetResource<Material>("Materials/telekinesis.xml"));
		glowNode->SetScale(1.1f);
	}
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Debris, OnCollision));
}

void Debris::FixedUpdate(float timeStep)
{
	if (body->GetLinearVelocity().Length() < 4.0f && node_->GetParent() == scene) //If it isn't parented to the scene, it must be being held by a PostalPope
	{
		Die();
	}
	if (dieTimer > 0.0f) 
	{
		if (smokeNode) { smokeNode->SetWorldPosition(node_->GetWorldPosition() ); }
		dieTimer -= timeStep;
		if (dieTimer <= 0.0f)
		{
			glowNode->Remove();
			node_->Remove();
		}
	}
}

void Debris::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (otherBody->GetCollisionLayer() & 128 && node_->GetParent() == scene)
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
