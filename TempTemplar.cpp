#include "TempTemplar.h"
#include "Urho3D/Core/Context.h"
#include <Urho3D\Graphics/StaticModel.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Audio/Sound.h>

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
	shield = scene->CreateChild("shield");
	shieldModel = shield->CreateComponent<StaticModel>();
	shieldModel->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
	shieldModel->SetMaterial(cache->GetResource<Material>("Materials/shield.xml"));
	shieldModel->SetEnabled(false);
	shield->SetScale(0.5f);
	WeakChild::MakeWeakChild(shield, node_);
	subShield = shield->CreateChild("subshield");
	StaticModel* subModel = (StaticModel*)subShield->CloneComponent(shieldModel);
	subShield->SetScale(0.95f);
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
		PODVector<RigidBody*> pushees;
		physworld->GetRigidBodies(pushees, Sphere(node_->GetWorldPosition(), shield->GetScale().x_ * 0.5f), 228);//4+32+64+128
		for (PODVector<RigidBody*>::Iterator i = pushees.Begin(); i != pushees.End(); ++i)
		{
			RigidBody* rb = dynamic_cast<RigidBody*>(*i);
			if (rb)
			{
				Node* n = rb->GetNode();
				if (n->HasComponent<Actor>() && n != node_)
				{
					Actor* a = n->GetComponent<Actor>();
					const Vector3 diff = n->GetWorldPosition() - node_->GetWorldPosition();
					Quaternion direction = Quaternion();
					direction.FromLookRotation(diff.Normalized(), Vector3::UP);
					a->KnockBack(12.0f + ((1.0f / diff.LengthSquared()) * 12.0f), direction);
				}
			}
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

TempTemplar::~TempTemplar()
{
}
