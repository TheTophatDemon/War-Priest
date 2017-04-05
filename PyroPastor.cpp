#include "PyroPastor.h"
#include "Urho3D/Core/Context.h"

#include "Gameplay.h"
#include "Actor.h"

#define STATE_DEAD 0
#define STATE_WANDER 1
#define STATE_ATTACK 32

PyroPastor::PyroPastor(Context* context) : Enemy(context)
{

}

void PyroPastor::DelayedStart()
{
	modelNode->SetRotation(Quaternion(-90.0f, Vector3::UP));
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
		turnTimer += 1;
		if (turnTimer > 50)
		{
			turnTimer = 0;
			newRotation = Quaternion(node_->GetRotation().y_ + Random(0, 360), Vector3::UP);
			walking = false;
			if (Random(0, 2) == 1) walking = true;
		}
		if (walking)
		{
			if (CheckCliff())
			{
				newRotation = Quaternion(node_->GetRotation().y_ + Random(90, 180), Vector3::UP);
				turnTimer = 0;
				walking = false;
			}
		}
		stateTimer += 1;
		/*if (stateTimer > 500)
		{
			ChangeState(STATE_ATTACK);
		}*/
		actor->Move(walking, false, false, false, false, deltaTime);
		break;
	case STATE_ATTACK:

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
