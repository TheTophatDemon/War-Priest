#include "Enemy.h"
#include "Urho3D/Physics/CollisionShape.h"
#include "Urho3D/Core/Context.h"
#include <iostream>

#include "Gameplay.h"
#include "Actor.h"
#include "Projectile.h"

#define STATE_DEAD 0
#define STATE_WANDER 1

Enemy::Enemy(Context* context) : LogicComponent(context)
{
	turnAmount = 0.0f; distanceFromPlayer = 0.0f; deltaTime = 0.0f;
}

void Enemy::RegisterObject(Context* context)
{
	context->RegisterFactory<Enemy>();
}

void Enemy::Start()
{
	game = GetScene()->GetComponent<Gameplay>();
	cache = GetSubsystem<ResourceCache>();
	scene = GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	body = node_->GetComponent<RigidBody>();
	modelNode = node_->GetChild("model");
	shape = node_->GetComponent<CollisionShape>();
	oldShape = new CollisionShape(context_);
	oldShape->SetSize(shape->GetSize());
	oldShape->SetPosition(shape->GetPosition());

	if (!node_->HasComponent<Actor>())
		actor = node_->CreateComponent<Actor>();
	else
		actor = node_->GetComponent<Actor>();
	actor->SetEnabled(false);

	state = -1;
	ChangeState(STATE_DEAD);
}

void Enemy::DelayedStart()
{
	modelNode->SetRotation(Quaternion(-90.0f, Vector3::UP));
	target = game->playerNode;

	node_->Rotate(Quaternion(Random(0.0f, 360.0f), Vector3::UP), TS_LOCAL);
	newRotation = node_->GetWorldRotation();
}

void Enemy::Execute()
{
}

void Enemy::FixedUpdate(float timeStep)
{
	deltaTime = timeStep;
	Vector3 plyPos = game->playerNode->GetWorldPosition(); plyPos.y_ = 0.0f;
	Vector3 ourPos = node_->GetWorldPosition(); ourPos.y_ = 0.0f;
	distanceFromPlayer = (ourPos - plyPos).Length();
	if (distanceFromPlayer < 60.0f)
	{
		Execute();
		node_->SetWorldRotation(node_->GetWorldRotation().Slerp(newRotation, 0.25f));
	}
	else
	{
		body->SetLinearVelocity(Vector3::ZERO);
	}
	if (node_->GetWorldPosition().y_ < game->waterHeight)
	{
		node_->Remove();
	}
}

void Enemy::EndFrameCheck(StringHash eventType, VariantMap& eventData)
{
	if (distanceFromPlayer < 60.0f)
	{
		body->SetEnabled(true);
	}
	else
	{
		body->SetEnabled(false);
	}
}

void Enemy::Wander()
{
	turnTimer += deltaTime;
	if (turnTimer > 0.6f)
	{
		turnTimer = 0.0f;
		turnAmount = (float)Random(-180, 180);
		if (turnAmount != 0.0f)
			newRotation = Quaternion(node_->GetWorldRotation().y_ + turnAmount, Vector3::UP);
		walking = true;
	}
	if (walking)
	{
		if (CheckCliff())
		{
			walking = false;
			turnTimer = 0.0f;
		}
		else
		{
			PhysicsRaycastResult result;
			physworld->RaycastSingle(result, Ray(node_->GetWorldPosition() + Vector3(0.0f, 0.1f, 0.0f), node_->GetDirection()), 2.0f, 2U);
			if (result.body_)
			{
				if (result.normal_.y_ == 0.0f) walking = false;
			}
		}
	}

	actor->SetMovement(walking, false, false, false);
	actor->Move(deltaTime);
}

void Enemy::Dead() //This function defines the defualt behavior for being dead
{
	PhysicsRaycastResult result;
	physworld->RaycastSingle(result, Ray(node_->GetWorldPosition() + Vector3(0.0f, 0.1f, 0.0f), Vector3::DOWN), 500.0f, 2);
	if (result.body_)
	{
		node_->SetWorldPosition(result.position_);
	}
	actor->SetEnabled(false);
	body->SetLinearVelocity(Vector3::ZERO);
	body->SetAngularVelocity(Vector3::ZERO);
}

void Enemy::Revive()
{
	node_->Translate(Vector3(0.0f, 0.1f, 0.0f), TS_LOCAL);
}

void Enemy::ChangeState(int newState)
{
	if (newState != state)
	{
		EnterState(newState);
		LeaveState(state);
		stateTimer = 0.0f;
	}
	state = newState;
}

void Enemy::EnterState(int newState)
{
	if (newState == STATE_DEAD)
	{
		shape->SetSize(Vector3(oldShape->GetSize().y_, oldShape->GetSize().x_ * 0.5f, oldShape->GetSize().y_));
		shape->SetPosition(Vector3(0.0f, -oldShape->GetSize().x_ * 0.5f, 0.0f));
		body->SetMass(0.0f);
	}
}

void Enemy::LeaveState(int oldState)
{
	if (oldState == STATE_DEAD)
	{
		actor->SetEnabled(true);
		shape->SetSize(oldShape->GetSize());
		shape->SetPosition(oldShape->GetPosition());
		body->SetMass(120.0f);
	}
}

void Enemy::FaceTarget()
{
	Quaternion face = Quaternion();
	Vector3 diff = (target->GetWorldPosition() - node_->GetWorldPosition()).Normalized();
	diff.y_ = 0.0f;
	face.FromLookRotation(diff, Vector3::UP);
	newRotation = face;
}

bool Enemy::CheckCliff()
{
	PhysicsRaycastResult result;
	physworld->RaycastSingle(result, Ray(node_->GetWorldPosition() + Vector3(0.0f, 0.2f, 0.0f) + node_->GetWorldDirection() * shape->GetSize().x_ * 2.5f, Vector3::DOWN), 2.0f, 2);
	if (!result.body_)
	{
		newRotation = Quaternion(node_->GetWorldRotation().EulerAngles().y_ + 180.0f, Vector3::UP);
		return true;
	}
	else
	{
		return false;
	}
}

Enemy::~Enemy()
{
	
}
