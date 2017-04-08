#include "PyroPastor.h"
#include "Urho3D/Core/Context.h"

#include "Gameplay.h"
#include "Actor.h"
#include <iostream>

#define STATE_DEAD 0
#define STATE_WANDER 1
#define STATE_ATTACK 32

PyroPastor::PyroPastor(Context* context) : Enemy(context)
{
	
}

void PyroPastor::DelayedStart()
{
	modelNode->SetRotation(Quaternion(-90.0f, Vector3::UP));
	actor->maxspeed = 10.0f;
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
			turnAmount = Random(0, 360);
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
		stateTimer += 1;
		if (stateTimer > 100)
		{
			ChangeState(STATE_ATTACK);
		}
		actor->Move(walking, false, false, false, false, deltaTime);
		break;
	case STATE_ATTACK:
		actor->Move(false, false, false, false, false, deltaTime);

		Quaternion face = Quaternion();
		Vector3 diff = (game->playerNode->GetWorldPosition() - node_->GetWorldPosition()).Normalized();
		diff.y_ = 0.0f;
		face.FromLookRotation(diff, Vector3::UP);
		newRotation = face;

		stateTimer += 1;
		if (stateTimer == 10 && distanceFromPlayer < 40.0f)
		{
			Quaternion aim = Quaternion();
			aim.FromLookRotation((game->playerNode->GetWorldPosition() - node_->GetWorldPosition()).Normalized(), Vector3::UP);
			game->MakeProjectile("fireball", node_->GetWorldPosition() + Vector3(0.0f, 1.5f, 0.0f), aim, node_);
		}
		if (stateTimer > 50)
		{
			ChangeState(STATE_WANDER);
		}
		break;
	}
}

void PyroPastor::Dead()
{
	Enemy::Dead();
	modelNode->SetRotation(Quaternion(90.0f, Vector3::RIGHT));
}

void PyroPastor::ChangeState(int newState)
{
	if (newState != STATE_DEAD && state == STATE_DEAD)
	{
		modelNode->SetRotation(Quaternion(-90.0f, Vector3::UP));
	}
	Enemy::ChangeState(newState);
}

PyroPastor::~PyroPastor()
{
}
