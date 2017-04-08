#include "Enemy.h"
#include "Urho3D/Core/Context.h"
#include <iostream>

#include "Gameplay.h"
#include "Actor.h"
#include "Projectile.h"

#define STATE_DEAD 0
#define STATE_WANDER 1

Enemy::Enemy(Context* context) : LogicComponent(context)
{
	turnAmount = 0.0f;
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
	actor->SetEnabled(false);

	state = STATE_DEAD;
}

void Enemy::Execute()
{
}

void Enemy::FixedUpdate(float timeStep)
{
	deltaTime = timeStep;
	float dist = (node_->GetWorldPosition() - game->playerNode->GetWorldPosition()).Length();
	if (dist < 50.0f)
	{
		body->SetEnabled(true);
		Execute();
		node_->SetRotation(node_->GetRotation().Slerp(newRotation, 0.25f));
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

void Enemy::Dead() //This function defines the defualt behavior for being dead
{
	actor->SetEnabled(false);
	body->SetLinearVelocity(Vector3::ZERO);
	body->SetAngularVelocity(Vector3::ZERO);
}

void Enemy::Revive()
{
	ChangeState(STATE_WANDER);
}

void Enemy::ChangeState(int newState)
{
	//For handling transitions

	if (newState != STATE_DEAD && state == STATE_DEAD)
	{
		actor->SetEnabled(true);
	}
	state = newState;
	stateTimer = 0;
}

bool Enemy::CheckCliff()
{
	PhysicsRaycastResult result;
	physworld->RaycastSingle(result, Ray(node_->GetWorldPosition() + (node_->GetRotation() * (Vector3::FORWARD * 2.0f)), Vector3::DOWN), 4, 2);
	if (!result.body_)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Enemy::OnHurt(Node* source, int amount)
{
	std::cout << "WAH! FORGET THIS PLACE!" << std::endl;
}

Enemy::~Enemy()
{
}
