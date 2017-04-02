#include "Enemy.h"
#include "Urho3D/Core/Context.h"

#include "Gameplay.h"
#include "Actor.h"

#define STATE_DEAD 0
#define STATE_WANDER 1

Enemy::Enemy(Context* context) : LogicComponent(context)
{
}

void Enemy::RegisterObject(Context* context)
{
	context->RegisterFactory<Enemy>();
}

void Enemy::Start()
{
	game = GetScene()->GetComponent<Gameplay>();
	cache = GetSubsystem<ResourceCache>();
	physworld = GetScene()->GetComponent<PhysicsWorld>();
	body = node_->GetComponent<RigidBody>();
	modelNode = node_->GetChild("model");

	if (!node_->HasComponent<Actor>())
		actor = node_->CreateComponent<Actor>();
	else
		actor = node_->GetComponent<Actor>();

	state = STATE_DEAD;
}

void Enemy::FixedUpdate(float timeStep)
{
	deltaTime = timeStep;
	float dist = (node_->GetWorldPosition() - game->playerNode->GetWorldPosition()).Length();
	if (dist < 50.0f)
	{
		body->SetEnabled(true);
		switch (state)
		{
		case STATE_DEAD:
			actor->SetEnabled(false);
			Dead();
			break;
		case STATE_WANDER:
			Wander();
			break;
		}
	}
	else
	{
		body->SetEnabled(false);
	}
}

void Enemy::Wander()
{
	actor->Move(true, false, false, false, false, deltaTime);
}

void Enemy::Dead()
{
	body->SetLinearVelocity(Vector3::ZERO);
	body->SetAngularVelocity(Vector3::ZERO);
}

void Enemy::ChangeState(int newState)
{
	//For handling transitions
	if (newState != STATE_DEAD && state == STATE_DEAD)
	{
		actor->SetEnabled(true);
	}
	state = newState;
}

Enemy::~Enemy()
{
}
