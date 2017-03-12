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

using namespace Urho3D;

#define LEVELMASK 2

Actor::Actor(Context* context) : LogicComponent(context)
{
	acceleration = 2.0f;
	maxspeed = 15.0f;
	friction = 0.85f;
	fallspeed = 0.4f;
	maxfall = 120.0f;
	jumpStrength = 12.0f;

	onGround = false;
	slopeSteepness = 0.0f;
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
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Actor, OnCollision));
}

void Actor::Move(bool fw, bool bk, bool rg, bool lf, bool jmp, float timeStep) 
{
	if (fw)
	{
		forward += acceleration;
		if (forward > maxspeed) forward = maxspeed;
	}
	else if (bk)
	{
		forward -= acceleration;
		if (forward < -maxspeed) forward = -maxspeed;
	}
	else
	{
		forward *= friction;
		if (fabs(forward) < 0.1f) forward = 0.0f;
	}

	if (rg)
	{
		strafe += acceleration;
		if (strafe > maxspeed) strafe = maxspeed;
	}
	else if (lf)
	{
		strafe -= acceleration;
		if (strafe < -maxspeed) strafe = -maxspeed;
	}
	else
	{
		strafe *= friction;
		if (fabs(strafe) < 0.1f) strafe = 0.0f;
	}

	fall -= fallspeed;
	if (fall < -maxfall) fall = -maxfall;
	if (onGround)
	{
		if (slopeSteepness != 0.75f)
		{
			fall = ((-1 / slopeSteepness) + 1) * maxspeed;
		}
		else
		{
			fall = 0.0f;
		}
	}
	if (jmp && onGround)
	{
		fall = jumpStrength;
	}
	else if (!jmp && fall > 0.0f)
	{
		fall -= 0.5f;
	}
	if (forward > 0.0f) StairCheck();

	Vector3 movement = (node_->GetRotation() * Vector3(strafe, fall, forward) * timeStep * 50.0f);
	body->SetLinearVelocity(movement);
	onGround = false;
	slopeSteepness = 0.75f;
	GetSlope();
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
		slopeSteepness = result.normal_.y_ * 0.75f;
	}
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
			if (fabs(normal.y_) != 0.0f && distance <= 0.0f && impulse != 0.0f)
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

Actor::~Actor()
{
}
