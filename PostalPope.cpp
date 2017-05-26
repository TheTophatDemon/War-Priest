#include "PostalPope.h"
#include "Urho3D/Core/Context.h"

#include "Gameplay.h"
#include "Actor.h"
#include "Projectile.h"
#include <iostream>

#define STATE_DEAD 0
#define STATE_WANDER 1
#define STATE_SUMMON 32
#define STATE_THROW 33

PostalPope::PostalPope(Context* context) : Enemy(context)
{
	deltaTime = 0.0f;
}

void PostalPope::DelayedStart()
{
	Enemy::DelayedStart();
	actor->maxspeed = 6.0f;
}

void PostalPope::RegisterObject(Context* context)
{
	context->RegisterFactory<PostalPope>();
}

void PostalPope::Execute()
{
	switch (state)
	{
	case STATE_DEAD:
		Dead();
		break;
	case STATE_WANDER:
		Wander();
		break;

	}
}

void PostalPope::Dead()
{
	Enemy::Dead();
}

void PostalPope::EnterState(int newState)
{
	Enemy::EnterState(newState);
}

void PostalPope::LeaveState(int oldState)
{
	Enemy::LeaveState(oldState);
}

void PostalPope::Revive()
{
	Enemy::Revive();
	ChangeState(STATE_WANDER);
}

PostalPope::~PostalPope()
{
}
