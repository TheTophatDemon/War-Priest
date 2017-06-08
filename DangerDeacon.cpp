#include "DangerDeacon.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsUtils.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <iostream>

#include "Actor.h"
#include "TempEffect.h"

#define STATE_DEAD 0
#define STATE_WANDER 1
#define STATE_CHASE 32
#define STATE_EXPLODE 33

#define BLASTRANGE 20.0f
#define STUNTIME 1.0f

DangerDeacon::DangerDeacon(Context* context) : Enemy(context)
{
	strafeAmt = 0.0f;
}

void DangerDeacon::DelayedStart()
{
	Enemy::DelayedStart();
	actor->maxspeed = 15.2f;
	orbThing = node_->CreateChild();
	orbModel = orbThing->CreateComponent<StaticModel>();
	orbModel->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
	orbModel->SetMaterial(cache->GetResource<Material>("Materials/fireglow.xml"));
	orbModel->SetViewMask(0);
	orbThing->SetScale(BLASTRANGE);
	orbThing->SetPosition(Vector3(0.0f, 1.5f, 0.0f));
}

void DangerDeacon::RegisterObject(Context* context)
{
	context->RegisterFactory<DangerDeacon>();
}

void DangerDeacon::Execute()
{
	float targetDist = 0.0f;
	if (target)
	{
		targetDist = (target->GetWorldPosition() - node_->GetWorldPosition()).Length();
	}
	switch (state)
	{
	case STATE_DEAD:
		Dead();
		break;
	case STATE_WANDER:
		Wander();
		if (target)
		{
			if (targetDist < 29.0f)
			{
				ChangeState(STATE_CHASE);
			}
		}
		break;
	case STATE_CHASE:
		if (target && targetDist < 30.0f)
		{
			const Vector3 headHeight = Vector3(0.0f, 2.0f, 0.0f);
			const Vector3 ourHead = node_->GetWorldPosition() + headHeight;
			const Vector3 footHeight = Vector3(0.0f, 0.5f, 0.0f);
			const Vector3 ourFeet = node_->GetWorldPosition() + footHeight;
			Vector3 dir = (target->GetWorldPosition() + headHeight - ourHead).Normalized();
			dir.y_ = 0.0f;
			
			//Raycast to the target from "head" and "foot" level. 
			PhysicsRaycastResult headCast;
			physworld->RaycastSingle(headCast, Ray(ourHead, dir), 100.0f, 210);//2+128+64+16
			PhysicsRaycastResult footCast;
			physworld->RaycastSingle(footCast, Ray(ourFeet + dir + Vector3::UP, Vector3::DOWN), 1.0f, 210);
			
			strafeAmt *= 0.9f;
			if (fabs(strafeAmt) < 0.1f) 
				strafeAmt = 0.0f;

			bool canSeePlayer = true;
			//If it finds something at head level, it's a bigger obstacle that needs to be dodged horizontally;
			if (headCast.body_)
			{
				if (!(headCast.body_->GetCollisionLayer() & 128) && headCast.distance_ < 2.0f)
				{
					if (strafeAmt == 0.0f) strafeAmt = (Random() - 0.5f) * 2.0f;
					canSeePlayer = false;
				}
			}
			if (canSeePlayer && footCast.body_) //If it's only at foot level, we can jump over it.
			{
				if (footCast.body_->GetCollisionLayer() & 2 && footCast.distance_ < 0.95f && footCast.normal_.y_ != 0.0f)
				{
					actor->Jump();
				}
			}
			
			//Gotta look the right way, wanker.
			if (canSeePlayer) 
			{
				Quaternion nRot = Quaternion();
				nRot.FromLookRotation(dir, Vector3::UP);
				newRotation = nRot;
			}

			actor->SetMovement(true, false, strafeAmt < 0.0f, strafeAmt > 0.0f);
			actor->Move(deltaTime);

			if (targetDist < 6.0f)
			{
				ChangeState(STATE_EXPLODE);
			}
		}
		else
		{
			ChangeState(STATE_WANDER);
		}
		break;
	case STATE_EXPLODE:
		stateTimer += deltaTime;
		
		const float shrinkAmount = deltaTime * BLASTRANGE * 4.0f;
		orbThing->SetScale(orbThing->GetScale() - Vector3(shrinkAmount, shrinkAmount, shrinkAmount));
		if (orbThing->GetScale().x_ <= 0.0f && orbModel->GetViewMask() != 0)
		{
			orbModel->SetViewMask(0);
			for (int i = 0; i < 5; ++i)
			{
				Vector3 pos = node_->GetWorldPosition() + Vector3(0.0f, 1.5f, 0.0f);
				pos.x_ += cosf(i * 1.2566f) * 2.5f;
				pos.z_ += sinf(i * 1.2566f) * 2.5f;
				Node* explosion = scene->CreateChild();
				explosion->SetWorldPosition(pos);
				ParticleEmitter* emitter = explosion->CreateComponent<ParticleEmitter>();
				emitter->SetEffect(cache->GetResource<ParticleEffect>("Particles/explosion.xml"));
				TempEffect* te = explosion->CreateComponent<TempEffect>();
				te->life = 2.0f;
			}
		}

		actor->SetMovement(false, false, false, false);
		actor->Move(deltaTime);

		if (stateTimer > STUNTIME)
		{
			ChangeState(STATE_WANDER);
		}
		break;
	}
}

void DangerDeacon::Dead()
{
	Enemy::Dead();
}

void DangerDeacon::EnterState(const int newState)
{
	Enemy::EnterState(newState);
	if (newState == STATE_EXPLODE)
	{
		orbModel->SetViewMask(-1);
		orbThing->SetScale(BLASTRANGE);
	}
}

void DangerDeacon::LeaveState(const int oldState)
{
	Enemy::LeaveState(oldState);
	if (oldState == STATE_EXPLODE)
	{
		orbModel->SetViewMask(0);
	}
}

void DangerDeacon::Revive()
{
	Enemy::Revive();
	ChangeState(STATE_WANDER);
}

DangerDeacon::~DangerDeacon()
{
}
