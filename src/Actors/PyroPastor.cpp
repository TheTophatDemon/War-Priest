/*
Copyright (C) 2021 Alexander Lunsford

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "PyroPastor.h"
#include "Urho3D/Core/Context.h"
#include <Urho3D/Graphics/Animation.h>

#include "../Gameplay.h"
#include "../Settings.h"
#include "../Actors/Actor.h"
#include "../Projectiles/Fireball.h"
#include <iostream>

#define IDLE_ANIM "Models/enemy/pyropastor_idle.ani"
#define REVIVE_ANIM "Models/enemy/pyropastor_revive.ani"
#define WALK_ANIM "Models/enemy/pyropastor_walk.ani"
#define ATTACK_ANIM "Models/enemy/pyropastor_attack.ani"

PyroPastor::PyroPastor(Context* context) : Enemy(context),
	shotCount(0),
	shotOffset(Random(-0.2f, 0.2f))
{
}

void PyroPastor::DelayedStart()
{
	cache->GetResource<Animation>(IDLE_ANIM);
	cache->GetResource<Animation>(REVIVE_ANIM);
	cache->GetResource<Animation>(WALK_ANIM);
	cache->GetResource<Animation>(ATTACK_ANIM);
	cache->GetResource<Material>("Materials/particle_fire.xml");
	cache->GetResource<Sound>("Sounds/enm_fireball.wav");

	Enemy::DelayedStart();

	animController->PlayExclusive(REVIVE_ANIM, 0, true, 0.0f);
	animController->SetSpeed(REVIVE_ANIM, 0.0f);

	actor->maxSpeed = 10.0f;
}

void PyroPastor::RegisterObject(Context* context)
{
	context->RegisterFactory<PyroPastor>();
}

void PyroPastor::Execute()
{
	Vector3 aimVec = Vector3::ZERO;
	Quaternion aim = Quaternion();
	if (target.Get())
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
		actor->SetInputFPS(walking, false, false, false);
		actor->Move(deltaTime);

		stateTimer += deltaTime;
		if (stateTimer > Settings::ScaleWithDifficulty(1.5f, 1.0f, 0.5f) + shotOffset)
		{
			//Check if player is in range
			PhysicsRaycastResult result;
			physworld->RaycastSingle(result, Ray(node_->GetWorldPosition() + Vector3(0.0f, 2.0f, 0.0f), aimVec), 60.0f, 130);//128+2
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
		if (stateTimer > 0.26f && shotCount == 0)
		{
			++shotCount;
			Fireball::MakeFireball(scene, node_->GetWorldPosition() + Vector3(0.0f, 2.0f, 0.0f), aim, node_);
		}
		else if (stateTimer > 0.46f && shotCount == 1 && Settings::GetDifficulty() > Settings::UNHOLY_THRESHOLD)
		{
			++shotCount;
			Fireball::MakeFireball(scene, node_->GetWorldPosition() + Vector3(0.0f, 2.0f, 0.0f), aim, node_);
		}
		if (stateTimer > 0.66f)
		{
			ChangeState(STATE_WANDER);
		}

		actor->SetInputFPS(false, false, false, false);
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

void PyroPastor::EnterState(const int newState)
{
	Enemy::EnterState(newState);
	if (newState == STATE_ATTACK)
	{
		shotCount = 0;
		soundSource->Play("Sounds/enm_fireball.wav");
	}
	else if (newState == STATE_IDLE)
	{
		animController->StopAll();
		animController->PlayExclusive(IDLE_ANIM, 0, false, 0.2f);
	}
}

void PyroPastor::LeaveState(const int oldState)
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
