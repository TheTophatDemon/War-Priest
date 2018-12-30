#include "RobeLocksMan.h"
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Animation.h>

#include "RobeLocksMissile.h"

#define STATE_DEAD 0 
#define STATE_WANDER 1

#define REVIVE_ANIM "Models/enemy/robelocksman_revive.ani"
#define IDLE_ANIM "Models/enemy/robelocksman_idle.ani"
#define WALK_ANIM "Models/enemy/robelocksman_walk.ani"
#define JUMP_ANIM "Models/enemy/robelocksman_jump.ani"

RobeLocksMan::RobeLocksMan(Context* context) : Enemy(context),
	shootTimer(Random(5.0f))
{
}

void RobeLocksMan::RegisterObject(Context* context)
{
	context->RegisterFactory<RobeLocksMan>();
}

void RobeLocksMan::DelayedStart()
{
	cache->GetResource<Animation>(REVIVE_ANIM);
	cache->GetResource<Animation>(IDLE_ANIM);
	cache->GetResource<Animation>(WALK_ANIM);
	cache->GetResource<Animation>(JUMP_ANIM);

	soundSource = node_->CreateComponent<SoundSounder>();

	Enemy::DelayedStart();
}

void RobeLocksMan::Revive()
{
	Enemy::Revive();
	animController->PlayExclusive(REVIVE_ANIM, 0, false, 0.2f);
	animController->SetSpeed(REVIVE_ANIM, 1.0f);
}

void RobeLocksMan::Execute()
{
	switch (state)
	{
	case STATE_DEAD:
		Dead();
		break;
	case STATE_WANDER:
		stateTimer += deltaTime;

		shootTimer -= deltaTime;
		if (shootTimer <= 0.0f)
		{
			shootTimer = Random(1.0f, 5.0f);
			RobeLocksMissile::MakeRobeLocksMissile(scene, node_->GetWorldPosition() + Vector3::UP * 1.5f, node_->GetWorldRotation(), node_);
			soundSource->Play("Sounds/enm_robelocksmissile.wav", true);
		}

		Wander(false, true);
		if (walking)
			animController->PlayExclusive(WALK_ANIM, 0, true, 0.2f);
		else
			animController->PlayExclusive(IDLE_ANIM, 0, true, 0.2f);
		break;
	}
}

void RobeLocksMan::Dead()
{
	Enemy::Dead();
	if (animController->IsAtEnd(REVIVE_ANIM))
	{
		ChangeState(STATE_WANDER);
	}
}

void RobeLocksMan::EnterState(const int newState)
{
	switch (newState)
	{
	case STATE_DEAD:
		animController->PlayExclusive(REVIVE_ANIM, 0, false, 0.0f);
		animController->SetSpeed(REVIVE_ANIM, 0.0f);
		break;
	}
}

void RobeLocksMan::LeaveState(const int oldState)
{
	
}

RobeLocksMan::~RobeLocksMan()
{
}
