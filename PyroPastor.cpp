#include "PyroPastor.h"
#include "Urho3D/Core/Context.h"

#include "Gameplay.h"
#include "Actor.h"
#include "Projectile.h"
#include <iostream>

#define STATE_DEAD 0
#define STATE_WANDER 1
#define STATE_ATTACK 32

#define IDLE_ANIM "Models/enemy/pyropastor_idle.ani"
#define REVIVE_ANIM "Models/enemy/pyropastor_revive.ani"
#define WALK_ANIM "Models/enemy/pyropastor_walk.ani"
#define ATTACK_ANIM "Models/enemy/pyropastor_attack.ani"

PyroPastor::PyroPastor(Context* context) : Enemy(context)
{
	deltaTime = 0.0f;
}

void PyroPastor::DelayedStart()
{
	Enemy::DelayedStart();

	animController->PlayExclusive(REVIVE_ANIM, 0, true, 0.0f);
	animController->SetSpeed(REVIVE_ANIM, 0.0f);

	actor->maxspeed = 10.0f;
}

void PyroPastor::RegisterObject(Context* context)
{
	context->RegisterFactory<PyroPastor>();
}

void PyroPastor::Execute()
{
	
	float turnAmount = 0.0f;
	Vector3 aimVec = Vector3::ZERO;
	Quaternion aim = Quaternion();
	if (target)
	{
		aimVec = (target->GetWorldPosition() - node_->GetWorldPosition()).Normalized();
		aim.FromLookRotation(aimVec, Vector3::UP);
	}
	
	switch (state)
	{
	case STATE_DEAD:
		Dead();
		break;
	case STATE_WANDER:
		Wander();

		stateTimer += deltaTime;
		if (stateTimer > 1.0f)
		{
			//Check if player is in range
			PhysicsRaycastResult result;
			physworld->RaycastSingle(result, Ray(node_->GetWorldPosition() + Vector3(0.0f, 2.0f, 0.0f), aimVec), 400.0f, 130);//128+2
			if (result.body_)
			{
				if (result.body_->GetCollisionLayer() & 128)
				{
					ChangeState(STATE_ATTACK);
				}
			}
			stateTimer = 0.0f;
		}

		//Select animation
		if (walking)
			animController->PlayExclusive(WALK_ANIM, 0, true, 0.2f);
		else
			animController->PlayExclusive(IDLE_ANIM, 0, true, 0.2f);

		break;
	case STATE_ATTACK:
		animController->PlayExclusive(ATTACK_ANIM, 0, false, 0.2f);

		FaceTarget();

		stateTimer += deltaTime;
		if (stateTimer > 0.26f && !shot)
		{
			shot = true;
			Projectile::MakeProjectile(scene, "fireball", node_->GetWorldPosition() + Vector3(0.0f, 2.0f, 0.0f), aim, node_); //Aim for the head or sliding is useless
		}
		if (stateTimer > 0.66f)
		{
			ChangeState(STATE_WANDER);
		}

		actor->SetMovement(false, false, false, false);
		actor->Move(deltaTime);
		break;
	}
}

void PyroPastor::Dead()
{
	Enemy::Dead();
	animController->PlayExclusive(REVIVE_ANIM, 0, true, 0.0f);
	if (animController->GetTime(REVIVE_ANIM) >= animController->GetLength(REVIVE_ANIM) * 0.9f)
	{
		ChangeState(STATE_WANDER);
	}
}

void PyroPastor::EnterState(int newState)
{
	Enemy::EnterState(newState);
	if (newState == STATE_ATTACK)
	{
		shot = false;
		soundSource->Play(cache->GetResource<Sound>("Sounds/enm_fireball.wav"));
	}
}

void PyroPastor::LeaveState(int oldState)
{
	Enemy::LeaveState(oldState);
	if (oldState == STATE_DEAD)
	{
		animController->SetSpeed(REVIVE_ANIM, 0.0f);
	}
}

void PyroPastor::Revive()
{
	Enemy::Revive();
	animController->PlayExclusive(REVIVE_ANIM, 0, false, 0.2f);
	animController->SetSpeed(REVIVE_ANIM, 1.4f);
}

PyroPastor::~PyroPastor()
{
}
