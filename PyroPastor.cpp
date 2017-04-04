#include "PyroPastor.h"
#include "Urho3D/Core/Context.h"

#include "Gameplay.h"
#include "Actor.h"

#define STATE_DEAD 0
#define STATE_WANDER 1

PyroPastor::PyroPastor(Context* context) : Enemy(context)
{

}

void PyroPastor::DelayedStart()
{
	
}

void PyroPastor::RegisterObject(Context* context)
{
	context->RegisterFactory<PyroPastor>();
}

void PyroPastor::Execute()
{
	switch (state)
	{
	case STATE_DEAD:
		Dead();
		break;
	case STATE_WANDER:
		actor->Move(true, false, false, false, true, deltaTime);
		break;
	}
}

void PyroPastor::ChangeState(int newState)
{
	Enemy::ChangeState(newState);
}

PyroPastor::~PyroPastor()
{
}
