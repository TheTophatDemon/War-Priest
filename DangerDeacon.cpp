#include "DangerDeacon.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsUtils.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/Animation.h>
#include <iostream>

#include "Actor.h"
#include "TempEffect.h"
#include "WeakChild.h"
#include "Projectile.h"
#include "Settings.h"
#include "Zeus.h"

#define STATE_DEAD 0
#define STATE_WANDER 1
#define STATE_CHASE 32
#define STATE_EXPLODE 33
#define STATE_POSE 34

#define EXPLODERANGE 16.0f
#define BLASTRANGE 5.5f
#define STUNTIME 1.3f
#define DAMAGE 12.0f

#define IDLE_ANIM "Models/enemy/dangerdeacon_idle.ani"
#define REVIVE_ANIM "Models/enemy/dangerdeacon_revive.ani"
#define WALK_ANIM "Models/enemy/dangerdeacon_walk.ani"
#define EXPLODE_ANIM "Models/enemy/dangerdeacon_explode.ani"
#define JUMP_ANIM "Models/enemy/dangerdeacon_jump.ani"
#define DROWN_ANIM "Models/enemy/dangerdeacon_drown.ani"

DangerDeacon::DangerDeacon(Context* context) : Enemy(context),
	strafeAmt(0.0f),
	fallTimer(0.0f)
{
}

void DangerDeacon::OnSettingsChange(StringHash eventType, VariantMap& eventData)
{
	actor->maxSpeed = 18.4f + Settings::ScaleWithDifficulty(-0.5f, 0.0f, 0.5f);
}

void DangerDeacon::DelayedStart()
{
	cache->GetResource<Animation>(IDLE_ANIM);
	cache->GetResource<Animation>(REVIVE_ANIM);
	cache->GetResource<Animation>(WALK_ANIM);
	cache->GetResource<Animation>(EXPLODE_ANIM);
	cache->GetResource<Animation>(JUMP_ANIM);
	cache->GetResource<Animation>(DROWN_ANIM);
	cache->GetResource<ParticleEffect>("Particles/explosion.xml");

	Enemy::DelayedStart();

	modelNode->SetParent(scene);
	WeakChild::MakeWeakChild(modelNode, node_);

	orbThing = node_->CreateChild();
	orbModel = orbThing->CreateComponent<StaticModel>();
	orbModel->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
	orbModel->SetMaterial(cache->GetResource<Material>("Materials/fireglow.xml"));
	orbModel->SetViewMask(0);
	orbThing->SetScale(EXPLODERANGE);
	orbThing->SetPosition(Vector3(0.0f, 1.5f, 0.0f));

	animController->PlayExclusive(REVIVE_ANIM, 0, true, 0.0f);
	animController->SetSpeed(REVIVE_ANIM, 0.0f);
	animController->SetSpeed(WALK_ANIM, 10.0f);

	SubscribeToEvent(Settings::E_SETTINGSCHANGED, URHO3D_HANDLER(DangerDeacon, OnSettingsChange));
	SendEvent(Settings::E_SETTINGSCHANGED);
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
	modelNode->SetWorldPosition(node_->GetWorldPosition());
	modelNode->SetWorldRotation(node_->GetWorldRotation());
	modelNode->Rotate(Quaternion(-90.0f, Vector3::UP));
	if (strafeAmt != 0.0f)
	{
		modelNode->Rotate(Quaternion(strafeAmt * 45.0f, Vector3::UP));
	}

	const float shrinkAmount = deltaTime * EXPLODERANGE * Settings::ScaleWithDifficulty(2.7f, 3.3f, 3.5f);

	//Falling animation
	if (!actor->onGround && 
		(!actor->downCast.body_ || (actor->downCast.body_ && actor->downCast.distance_ > 5.0f)) 
		&& actor->IsEnabled() && !animController->IsPlaying(JUMP_ANIM))
	{
		fallTimer += deltaTime;
		if (fallTimer > 0.25f)
		{
			animController->PlayExclusive(DROWN_ANIM, 255, true, 0.2f);
		}
	}
	else
	{
		fallTimer = 0.0f;
		animController->Stop(DROWN_ANIM, 0.2f);
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
			if (targetDist < 34.0f)
			{
				ChangeState(STATE_CHASE);
			}
		}
		if (walking)
			animController->PlayExclusive(WALK_ANIM, 0, true, 0.2f);
		else
			animController->PlayExclusive(IDLE_ANIM, 0, true, 0.2f);
		break;
	case STATE_CHASE:
		if (target && targetDist < 35.0f)
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
			physworld->RaycastSingle(footCast, Ray(ourFeet + dir + Vector3(0.0f, 1.5f, 0.0f), Vector3::DOWN), 1.0f, 210);
			
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
				if (footCast.body_->GetCollisionLayer() & 2 && footCast.distance_ < 1.45f && footCast.normal_.y_ != 0.0f)
				{
					actor->Jump();
					animController->Play(JUMP_ANIM, 128, false, 0.2f);
				}
			}

			if (Settings::GetDifficulty() > Settings::UNHOLY_THRESHOLD && footCast.distance_ > 3.0f)
			{
				actor->Jump();
				animController->Play(JUMP_ANIM, 128, false, 0.2f);
			}

			if (fabs(actor->fall) > 0.5f)
			{
				strafeAmt = 0.0f;
			}
			
			//Gotta look the right way, wanker.
			if (canSeePlayer) 
			{
				Quaternion nRot = Quaternion();
				nRot.FromLookRotation(dir, Vector3::UP);
				newRotation = nRot;
			}
			
			if (animController->IsAtEnd(JUMP_ANIM)) animController->Stop(JUMP_ANIM, 0.2f);
			animController->PlayExclusive(WALK_ANIM, 0, true, 0.2f);

			actor->SetInputFPS(true, false, strafeAmt < 0.0f, strafeAmt > 0.0f);
			actor->Move(deltaTime);

			if (targetDist < 6.0f && canSeePlayer)
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
		animController->PlayExclusive(EXPLODE_ANIM, 128, false, 0.2f);
		
		orbThing->SetScale(orbThing->GetScale() - Vector3(shrinkAmount, shrinkAmount, shrinkAmount));
		if (orbThing->GetScale().x_ <= 0.0f)
		{
			if (orbModel->GetViewMask() != 0) 
			{
				orbModel->SetViewMask(0);
				//The part where he explodes
				for (int i = 0; i < 6; ++i)
				{
					Vector3 pos = node_->GetWorldPosition() + Vector3(0.0f, 1.5f, 0.0f);
					pos.x_ += cosf(i * 1.0472f) * 3.5f;
					pos.z_ += sinf(i * 1.0472f) * 3.5f;
					Zeus::MakeExplosion(scene, pos, 2.0f);
				}
				soundSource->Play("Sounds/env_explode.wav");
			}
			//Apply damage to entities
			if (stateTimer < STUNTIME - 0.5f)
			{
				Zeus::ApplyRadialDamage(scene, node_, BLASTRANGE, DAMAGE, 132); //128 + 4
			}
		}

		actor->SetInputFPS(false, false, false, false);
		actor->Move(deltaTime);

		if (stateTimer > STUNTIME)
		{
			ChangeState(STATE_WANDER);
		}
		break;


	case STATE_POSE:
		stateTimer += deltaTime;
		if (stateTimer > animController->GetLength(REVIVE_ANIM) * 0.9f)
		{
			ChangeState(STATE_WANDER);
		}
		actor->SetInputFPS(false, false, false, false);
		actor->Move(deltaTime);
		break;
	}
}

void DangerDeacon::Dead()
{
	Enemy::Dead();
	animController->PlayExclusive(REVIVE_ANIM, 0, false, 0.0f);
	if (animController->GetTime(REVIVE_ANIM) >= animController->GetLength(REVIVE_ANIM) * 0.9f)
	{
		ChangeState(STATE_WANDER);
	}
}

void DangerDeacon::EnterState(const int newState)
{
	Enemy::EnterState(newState);
	if (newState == STATE_EXPLODE)
	{
		orbModel->SetViewMask(-1);
		orbThing->SetScale(EXPLODERANGE);
		animController->StopAll();
		soundSource->Play("Sounds/enm_fuse.wav");
	}
	else if (newState == STATE_CHASE)
	{
		animController->PlayExclusive(WALK_ANIM, 0, true, 0.2f);
	}
}

void DangerDeacon::LeaveState(const int oldState)
{
	Enemy::LeaveState(oldState);
	if (oldState == STATE_EXPLODE)
	{
		orbModel->SetViewMask(0);
		animController->StopAll();
	}
}

void DangerDeacon::Revive()
{
	Enemy::Revive();
	animController->PlayExclusive(REVIVE_ANIM, 0, false, 0.2f);
	animController->SetSpeed(REVIVE_ANIM, 1.0f);
	FaceTarget();
	ChangeState(STATE_POSE);
}

DangerDeacon::~DangerDeacon()
{
}
