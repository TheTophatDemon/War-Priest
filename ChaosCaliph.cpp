#include "ChaosCaliph.h"
#include "Urho3D/Core/Context.h"
#include <Urho3D\Graphics/StaticModel.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Math/Ray.h>

#include "Actor.h"
#include "Fireball.h"

#define SPIN_RANGE 7.0f

#define STATE_DEAD 0
#define STATE_WANDER 1
#define STATE_SHOOT 32
#define STATE_SPIN 33

ChaosCaliph::ChaosCaliph(Context* context) : Enemy(context), 
	shot(false),
	lastState(STATE_DEAD)
{
}

void ChaosCaliph::RegisterObject(Context* context)
{
	context->RegisterFactory<ChaosCaliph>();
}

void ChaosCaliph::DelayedStart()
{
	Enemy::DelayedStart();
	actor->maxspeed = 15.0f;
	actor->acceleration = 50.0f;
	actor->friction = 0.5f;
	actor->maxfall = 15.0f;
}

void ChaosCaliph::Execute()
{
	Vector3 aimVec = Vector3::ZERO;
	float targetDistance;
	if (target.Get())
	{
		const Vector3 tarDiff = (target->GetWorldPosition() - node_->GetWorldPosition());
		targetDistance = tarDiff.Length();
		aimVec = tarDiff.Normalized();
	}

	switch (state)
	{
	case STATE_DEAD:
		Dead();
		break;
	case STATE_WANDER:
		stateTimer += deltaTime;

		Wander(false, false);
		
		if (targetDistance < SPIN_RANGE && lastState != STATE_DEAD)
			ChangeState(STATE_SPIN);

		if (stateTimer > 1.0f)
		{
			//Check if player is within aim
			PhysicsRaycastResult result;
			physworld->RaycastSingle(result, Ray(node_->GetWorldPosition() + Vector3(0.0f, 2.0f, 0.0f), aimVec), 400.0f, 130);//128+2
			if (result.body_)
			{
				if (result.body_->GetCollisionLayer() & 128)
				{
					ChangeState(STATE_SHOOT);
				}
			}
			stateTimer = 0.0f;
		}

		break;
	case STATE_SHOOT:
		FaceTarget();
		
		stateTimer += deltaTime;
		if (stateTimer > 0.25f)
		{
			if (!shot)
			{
				shot = true;
				for (int i = -1; i <= 1; ++i)
				{
					if (i == 0) continue;
					Fireball::MakeFireball(scene, 
						node_->GetWorldPosition() + Vector3(0.0f, 2.0f, 0.0f) + (node_->GetWorldDirection() * 2.0f),
						Quaternion((i*30.0f) + node_->GetWorldRotation().EulerAngles().y_, Vector3::UP),
						node_);
				}
			}
			else
			{
				if (stateTimer > 0.5f) ChangeState(STATE_WANDER);
			}
		}
		
		actor->SetMovement(0.0f, 0.0f);
		actor->Move(deltaTime);
		break;
	case STATE_SPIN:
		stateTimer += deltaTime;

		if (targetDistance > SPIN_RANGE)
		{
			ChangeState(STATE_WANDER);
		}
		newRotation = Quaternion(stateTimer * 1080.0f, Vector3::UP);

		actor->SetMovement(aimVec * actor->maxspeed * 0.75f);
		actor->Move(deltaTime);
		break;
	}
}

void ChaosCaliph::EnterState(const int newState)
{
	lastState = state;
	Enemy::EnterState(newState);
	if (newState == STATE_SHOOT)
	{
		shot = false;
	}
}

void ChaosCaliph::LeaveState(const int oldState)
{
	Enemy::LeaveState(oldState);
}

void ChaosCaliph::Dead()
{
	Enemy::Dead();
}

void ChaosCaliph::Revive()
{
	Enemy::Revive();
	ChangeState(STATE_WANDER);
}

ChaosCaliph::~ChaosCaliph()
{
}
