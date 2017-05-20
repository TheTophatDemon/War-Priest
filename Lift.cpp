#include "Lift.h"

#include <Urho3D/Core/Context.h>

#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/PhysicsUtils.h>
#include <iostream>

Lift* Lift::MakeLiftComponent(Context* context, Vector3 mov, float rT, float spd, float rSpd)
{
	Lift* lift = new Lift(context);
	lift->movement = mov;
	lift->restTime = rT;
	lift->speed = spd;
	lift->rotateSpeed = rSpd;
	return lift;
}

Lift::Lift(Context* context) : LogicComponent(context)
{
}

void Lift::RegisterObject(Context* context)
{
	context->RegisterFactory<Lift>();
}

void Lift::Start()
{
	Vector3 pointA = node_->GetWorldPosition();
	Vector3 pointB = pointA + movement;
	valAnim = new ValueAnimation(context_);
	valAnim->SetKeyFrame(0.0f, pointA);
	valAnim->SetKeyFrame(restTime, pointA);
	valAnim->SetKeyFrame(restTime + speed, pointB);
	valAnim->SetKeyFrame((restTime * 2.0f) + speed, pointB);
	valAnim->SetKeyFrame((restTime + speed) * 2.0f, pointA);
	node_->SetAttributeAnimation("Position", valAnim, WM_LOOP, 1.0f);
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Lift, OnCollision));
}

void Lift::FixedUpdate(float timeStep)
{
	if (timer > 0.0f)
	{
		timer -= timeStep;
		if (timer <= 0)
		{
			timer = 0.0f;
			node_->SetAttributeAnimationSpeed("Position", 1.0f);
		}
	}
	else
	{
		node_->Rotate(Quaternion(rotateSpeed * timeStep, Vector3::UP), TS_LOCAL);
	}
}

void Lift::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (otherBody->GetCollisionLayer() & 128 || otherBody->GetCollisionLayer() & 64) //KNOCK KNOCK ITS ME
	{
		Vector3 normMovement;
		VectorBuffer contacts = eventData["Contacts"].GetBuffer();
		while (!contacts.IsEof())
		{
			Vector3 position = contacts.ReadVector3();
			Vector3 normal = contacts.ReadVector3();
			float distance = contacts.ReadFloat();
			float impulse = contacts.ReadFloat();
			if (fabs(normal.y_) >= 0.42f && position.y_ > node_->GetWorldPosition().y_)
			{
				return;
			}
		}
		node_->SetAttributeAnimationSpeed("Position", 0.0f);
		timer = 0.5f;
	}
}

Lift::~Lift()
{
}
