#include "ChaosCaliph.h"
#include "Urho3D/Core/Context.h"
#include <Urho3D\Graphics/StaticModel.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Math/Ray.h>

#include "Actor.h"

#define STATE_DEAD 0
#define STATE_WANDER 1
#define STATE_SHOOT 32

ChaosCaliph::ChaosCaliph(Context* context) : Enemy(context), 
	shot(false)
{
}

void ChaosCaliph::RegisterObject(Context* context)
{
	context->RegisterFactory<ChaosCaliph>();
}

void ChaosCaliph::DelayedStart()
{
	Enemy::DelayedStart();
	actor->maxspeed = 8.0f;
	actor->acceleration = 50.0f;
	actor->friction = 0.95f;
	actor->maxfall = 15.0f;
}

void ChaosCaliph::Execute()
{
	Vector3 aimVec = Vector3::ZERO;
	if (target.Get())
	{
		aimVec = (target->GetWorldPosition() - node_->GetWorldPosition()).Normalized();
	}

	switch (state)
	{
	case STATE_DEAD:
		Dead();
		break;
	case STATE_WANDER:
		Wander(false, true);
		
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
				projectile = Blackstone::MakeBlackstone(scene,
					node_->GetWorldPosition() + Vector3(0.0f, 2.0f, 0.0f) + (node_->GetWorldDirection() * 2.0f),
					(aimVec * body->GetMass() * 12.0f) + (Vector3::UP*body->GetMass()*(16.0f + (aimVec.y_ * 11.0f))),
					node_);
			}
			else
			{
				if (projectile.Get())
				{
					if (!projectile->IsChildOf(scene))
					{
						ChangeState(STATE_WANDER);
					}
				}
				else
				{
					ChangeState(STATE_WANDER);
				}
			}
		}
		
		actor->SetMovement(0.0f, 0.0f);
		actor->Move(deltaTime);
		break;
	}
}

void ChaosCaliph::EnterState(const int newState)
{
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
