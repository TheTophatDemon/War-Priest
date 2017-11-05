#include "ChaosCaliph.h"
#include "Urho3D/Core/Context.h"
#include <Urho3D\Graphics/StaticModel.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Graphics/Model.h>

#include "Actor.h"

#define STATE_DEAD 0
#define STATE_WANDER 1
#define STATE_SHOOT 32

ChaosCaliph::ChaosCaliph(Context* context) : Enemy(context)
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
	switch (state)
	{
	case STATE_DEAD:
		Dead();
		break;
	case STATE_WANDER:
		Wander(false, true);
		actor->Jump();
		break;
	case STATE_SHOOT:
		actor->SetMovement(0.0f, 0.0f);
		actor->Move(deltaTime);
		break;
	}
}

void ChaosCaliph::EnterState(const int newState)
{
	Enemy::EnterState(newState);
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
