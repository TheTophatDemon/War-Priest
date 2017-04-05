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
	float turnAmount = 0.0f;
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
			turnAmount = Random(0, 360);
			walking = false;
			if (Random(0, 2) == 1) walking = true;
		}
		if (walking)
		{
			if (CheckCliff())
			{
				turnAmount = Random(90, 180);
				turnTimer = 0;
				walking = false;
			}
			else
			{
				PhysicsRaycastResult result;
				physworld->RaycastSingle(result, Ray(node_->GetWorldPosition() + Vector3(0.0f, 1.5f, 0.0f), node_->GetWorldRotation() * Vector3::FORWARD), 4.0f, 2U);
				if (result.body_)
				{
					turnAmount = Random(90, 180);
				}
			}
		}
		if (turnAmount != 0.0f)
			newRotation = Quaternion(node_->GetRotation().y_ + turnAmount, Vector3::UP);
		stateTimer += 1;
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
