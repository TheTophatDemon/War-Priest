#include "TempTemplar.h"
#include "Urho3D/Core/Context.h"
#include <Urho3D\Graphics/StaticModel.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsEvents.h>

#include "Gameplay.h"
#include "Actor.h"
#include "Projectile.h"
#include "Zeus.h"
#include "WeakChild.h"
#include "GunPriest.h"
#include "Settings.h"
#include <iostream>

#define STATE_DEAD 0
#define STATE_WANDER 1

#define SHIELD_SIZE 18.0f

TempTemplar::TempTemplar(Context* context) : Enemy(context)
{
	deltaTime = 0.0f;
}

void TempTemplar::DelayedStart()
{
	Enemy::DelayedStart();
	actor->maxspeed = 6.0f;

	//Make shield
	shield = scene->CreateChild("shield");
	shield->AddTag("tempshield");
	CollisionShape* cs = shield->CreateComponent<CollisionShape>();
	cs->SetSphere(1.0f);
	RigidBody* rb = shield->CreateComponent<RigidBody>();
	rb->SetCollisionLayer(17);//1+16
	rb->SetTrigger(true);
	rb->SetLinearFactor(Vector3::ZERO);
	rb->SetAngularFactor(Vector3::ZERO);
	
	shieldModel = shield->CreateComponent<StaticModel>();
	shieldModel->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
	shieldModel->SetMaterial(cache->GetResource<Material>("Materials/shield.xml"));
	shieldModel->SetEnabled(false);
	shield->SetScale(0.5f);
	WeakChild::MakeWeakChild(shield, node_);
	subShield = shield->CreateChild("subshield");
	StaticModel* subModel = (StaticModel*)subShield->CloneComponent(shieldModel);
	subShield->SetScale(0.95f);

	SubscribeToEvent(shield, E_NODECOLLISION, URHO3D_HANDLER(TempTemplar, OnShieldCollision));

}

void TempTemplar::RegisterObject(Context* context)
{
	context->RegisterFactory<TempTemplar>();
}

void TempTemplar::Execute()
{
	shield->SetWorldPosition(node_->GetWorldPosition());
	shield->Rotate(Quaternion(deltaTime * 100.0f, Vector3::UP), TS_LOCAL);
	subShield->Rotate(Quaternion(deltaTime * 50.0f, Vector3::UP), TS_LOCAL);
	switch (state)
	{
	case STATE_DEAD:
		Dead();
		break;
	case STATE_WANDER:
		if (shield->GetScale().x_ < SHIELD_SIZE)
		{
			shield->SetScale(shield->GetScale() + Vector3(deltaTime * 10.0f, deltaTime * 10.0f, deltaTime * 10.0f));
		}
		else
		{
			shield->SetScale(SHIELD_SIZE);
		}
		Wander(false, true);
		break;
	}
}

void TempTemplar::Dead()
{
	Enemy::Dead();
}

void TempTemplar::EnterState(const int newState)
{
	Enemy::EnterState(newState);
	if (newState == STATE_WANDER)
	{
		shieldModel->SetEnabled(true);
		subShield->GetComponent<StaticModel>()->SetEnabled(true);
	}
}

void TempTemplar::LeaveState(const int oldState)
{
	Enemy::LeaveState(oldState);
}

void TempTemplar::Revive()
{
	Enemy::Revive();
	ChangeState(STATE_WANDER);
}

void TempTemplar::OnShieldCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();

	if (other != node_) 
	{
		if (other->HasComponent<Actor>())
		{
			const Vector3 diff = other->GetWorldPosition() - node_->GetWorldPosition();
			Quaternion direction = Quaternion();
			direction.FromLookRotation(diff.Normalized(), Vector3::UP);
			other->GetComponent<Actor>()->KnockBack(12.0f + (12.0f / diff.LengthSquared()), direction);
		}
		else if (otherBody->GetMass() > 0)
		{
			const Vector3 diff = other->GetWorldPosition() - node_->GetWorldPosition();
			const float push = 3.0f + (12.0f / diff.LengthSquared());
			otherBody->ApplyImpulse(diff.Normalized() * Vector3(push, push, push) * otherBody->GetMass());
		}
	}
}

TempTemplar::~TempTemplar()
{
}
