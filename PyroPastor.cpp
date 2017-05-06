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
	if (modelNode->HasComponent<AnimationController>())
		animController = modelNode->GetComponent<AnimationController>();
	else
		animController = modelNode->CreateComponent<AnimationController>();

	animController->PlayExclusive(REVIVE_ANIM, 0, true, 0.0f);
	animController->SetSpeed(REVIVE_ANIM, 0.0f);

	modelNode->SetRotation(Quaternion(-90.0f, Vector3::UP));
	actor->maxspeed = 10.0f;
	target = game->playerNode;

	node_->Rotate(Quaternion(Random(0.0f, 360.0f), Vector3::UP), TS_LOCAL);
	newRotation = node_->GetWorldRotation();
}

void PyroPastor::RegisterObject(Context* context)
{
	context->RegisterFactory<PyroPastor>();
}

void PyroPastor::Execute()
{
	float turnAmount = 0.0f;
	switch (state)
	{
	case STATE_DEAD:
		Dead();
		break;
	case STATE_WANDER:
		turnTimer += 1;
		if (turnTimer > 50)
		{
			turnTimer = 0;
			turnAmount = (float)Random(0, 360);
			walking = true;
		}
		if (walking)
		{
			if (CheckCliff())
			{
				walking = false;
			}
			else 
			{
				PhysicsRaycastResult result;
				physworld->RaycastSingle(result, Ray(node_->GetWorldPosition() + Vector3(0.0f, 0.05f, 0.0f), node_->GetRotation() * (Vector3::FORWARD * 2.0f)), 4.0f, 2U);
				if (result.body_)
				{
					walking = false;
				}
			}
		}

		if (turnAmount != 0.0f)
			newRotation = Quaternion(node_->GetRotation().y_ + turnAmount, Vector3::UP);
		stateTimer += deltaTime;
		if (stateTimer > 1.0f)
		{
			ChangeState(STATE_ATTACK);
		}
		actor->SetMovement(walking, false, false, false);
		actor->Move(deltaTime);

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
		if (stateTimer > 0.16f && distanceFromPlayer < 40.0f && !shot)
		{
			shot = true;
			Quaternion aim = Quaternion();
			aim.FromLookRotation((target->GetWorldPosition() + Vector3(0.0f, 1.0f, 0.0f) - node_->GetWorldPosition()).Normalized(), Vector3::UP);
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

void PyroPastor::ChangeState(int newState)
{
	if (newState != STATE_DEAD && state == STATE_DEAD)
	{
		animController->SetSpeed(REVIVE_ANIM, 0.0f);
	}
	if (newState == STATE_ATTACK && state != STATE_ATTACK)
	{
		shot = false;
	}
	Enemy::ChangeState(newState);
}

void PyroPastor::FaceTarget()
{
	Quaternion face = Quaternion();
	Vector3 diff = (target->GetWorldPosition() - node_->GetWorldPosition()).Normalized();
	diff.y_ = 0.0f;
	face.FromLookRotation(diff, Vector3::UP);
	newRotation = face;
}

void PyroPastor::Revive()
{
	node_->Translate(Vector3(0.0f, 0.1f, 0.0f), TS_LOCAL);
	animController->PlayExclusive(REVIVE_ANIM, 0, false, 0.2f);
	animController->SetSpeed(REVIVE_ANIM, 1.4f);
}

PyroPastor::~PyroPastor()
{
}
