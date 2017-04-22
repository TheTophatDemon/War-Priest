#include "Actor.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsUtils.h>
#include <Urho3D/Math/Ray.h>
#include <iostream>

using namespace Urho3D;

#define LEVELMASK 2
#define GROUNDMARGIN 0.51f

Actor::Actor(Context* context) : LogicComponent(context)
{
	acceleration = 150.0f;
	maxspeed = 15.0f;
	friction = 0.85f;
	fallspeed = 50.0f;
	maxfall = 150.0f;
	jumpStrength = 18.0f;

	onGround = false;
	slopeSteepness = 0.0f;
	aiState = 0;
	knockBack = 0.0f;
	deltaTime = 0.0f;

	forward = 0.0f;
	strafe = 0.0f;

	rawMovement = Vector3::ZERO;
	finalMovement = Vector3::ZERO;
}

void Actor::RegisterObject(Context* context)
{
	context->RegisterFactory<Actor>();
}

void Actor::Start()
{
	body = node_->GetComponent<RigidBody>();
	scene = node_->GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	shape = node_->GetComponent<CollisionShape>();
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Actor, OnCollision));
	//SubscribeToEvent(E_PHYSICSPRESTEP, URHO3D_HANDLER(Actor, PreStep));
}

void Actor::SetMovement(bool fw, bool bk, bool lf, bool rg)
{
	if (fabs(deltaTime) > 10.0f) deltaTime = 0.0f;
	if (fw)
	{
		forward += acceleration * deltaTime;
		if (forward > maxspeed) forward = maxspeed;
	}
	else if (bk)
	{
		forward -= acceleration * deltaTime;
		if (forward < -maxspeed) forward = -maxspeed;
	}
	else
	{
		forward *= friction;
		if (fabs(forward) < 0.1f) forward = 0.0f;
	}
	if (rg)
	{
		strafe += acceleration * deltaTime;
		if (strafe > maxspeed) strafe = maxspeed;
	}
	else if (lf)
	{
		strafe -= acceleration * deltaTime;
		if (strafe < -maxspeed) strafe = -maxspeed;
	}
	else
	{
		strafe *= friction;
		if (fabs(strafe) < 0.1f) strafe = 0.0f;
	}
	rawMovement = node_->GetWorldRotation() * Vector3(strafe, 0.0f, forward);
}

void Actor::SetMovement(float xm, float zm)
{
	rawMovement = Vector3(xm, 0.0f, zm);
}
void Actor::SetMovement(Vector3 mv)
{
	rawMovement = mv;
}

void Actor::Jump()
{
	if (onGround)
		fall = jumpStrength;
}

void Actor::Move(float timeStep)
{
	deltaTime = timeStep;
	if (fabs(deltaTime) > 10.0f) deltaTime = 0.0f;

	//Falling logic
	sloping = false;
	float slopeFall = 0.0f;
	if (onGround)
	{
		if (fall <= 0.0f) 
		{
			fall = -0.1f;
			if (slopeSteepness != 1.0f && slopeSteepness >= 0.42f)
			{
				slopeFall = (-1 / (slopeSteepness * 0.9f) + 1) * maxspeed;
				sloping = true;
			}
		}
	}
	else
	{
		fall -= fallspeed * deltaTime;
		if (fall < -maxfall) fall = -maxfall;
	}

	//Apply movements
	if (knockBack > 0.1f)
	{
		knockBack *= 0.9f;
	}
	else
	{
		knockBack = 0.0f;
	}
	
	finalMovement = ((Vector3(rawMovement.x_, fall + slopeFall, rawMovement.z_) + (knockBackDirection * Vector3::FORWARD * knockBack)) * deltaTime * 50.0f);
	body->SetLinearVelocity(finalMovement);
	rawMovement = Vector3::ZERO;
	
	slopeSteepness = 0.75f;
	GetSlope();

	onGround = false;
}

void Actor::KnockBack(float amount, Quaternion direction)
{
	knockBack = amount;
	knockBackDirection = direction;
}

void Actor::FixedUpdate(float timeStep)
{
	
}

void Actor::GetSlope()
{
	//Raycast downward to get slope normal
	PhysicsRaycastResult result;
	physworld->RaycastSingle(result, Ray(node_->GetWorldPosition() + Vector3(0.0f, 0.5f, 0.0f), Vector3::DOWN), 500.0f, LEVELMASK);
	if (result.body_)
	{
		slopeSteepness = result.normal_.y_;
	}
}

void Actor::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (otherBody->GetCollisionLayer() & LEVELMASK)
	{
		VectorBuffer contacts = eventData["Contacts"].GetBuffer();
		while (!contacts.IsEof())
		{
			Vector3 position = contacts.ReadVector3();
			Vector3 normal = contacts.ReadVector3();
			float distance = contacts.ReadFloat();
			float impulse = contacts.ReadFloat();
			if (fabs(normal.y_) != 0.0f)
			{
				if (position.y_ <= node_->GetPosition().y_ + 0.5f)
				{
					onGround = true;
				}
				if (position.y_ >= node_->GetPosition().y_ + 2.5f && fall > 0.0f && distance < 0.005f)
				{
					fall = 0.0f;
				}
			}
		}
	}
}

void Actor::PreStep(StringHash eventType, VariantMap& eventData)
{
	onGround = false;
}

void Actor::ChangeState(int newState)
{
	aiState = newState;
}

void Actor::StairCheck()
{
	//Raycast forward from feet. If something's there, do a second raycast from below to judge the height of the step.
	PhysicsRaycastResult result;
	Vector3 dir = node_->GetRotation() * Vector3::FORWARD;
	physworld->RaycastSingle(result, Ray(node_->GetWorldPosition() + (Vector3::UP * 0.05f), dir), 1.0f, LEVELMASK);
	if (result.body_ && result.normal_.y_ == 0.0f)
	{
		physworld->RaycastSingle(result, Ray(result.position_ + (dir * 0.1f), Vector3::UP), 10.0f, LEVELMASK);
		if (result.body_ && result.distance_ < 1.0f)
		{
			fall = 8.0f;
		}
	}
}

Actor::~Actor()
{
}


/*physworld->RaycastSingle(groundCasts[2], Ray(node_->GetWorldPosition() + offset + sideOffset, Vector3::DOWN), 500.0f, 2);
physworld->RaycastSingle(groundCasts[3], Ray(node_->GetWorldPosition() + offset - sideOffset, Vector3::DOWN), 500.0f, 2);
float maxY = -10000.0f;
for (int i = 0; i < 4; ++i)
{
if (groundCasts[i].body_)
{
if (groundCasts[i].position_.y_ > maxY)
maxY = groundCasts[i].position_.y_;
if (groundCasts[i].distance_ < GROUNDMARGIN)
{
onGround = true;
}
}
}*/