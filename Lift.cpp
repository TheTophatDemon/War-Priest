#include "Lift.h"

#include <Urho3D/Core/Context.h>

#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/PhysicsUtils.h>
#include <iostream>

Lift* Lift::MakeLiftComponent(Context* context, const Vector3 mov, const float rT, const float spd, const float rSpd, const float aRad)
{
	Lift* lift = new Lift(context);
	lift->movement = mov;
	lift->restTime = rT;
	lift->speed = spd;
	lift->rotateSpeed = rSpd;
	lift->activeRadius = aRad;
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
	game = GetScene()->GetComponent<Gameplay>();
	Vector3 pointA = node_->GetWorldPosition();
	Vector3 pointB = pointA + movement;
	valAnim = new ValueAnimation(context_);
	valAnim->SetInterpolationMethod(InterpMethod::IM_SPLINE);
	valAnim->SetSplineTension(0.1f);
	valAnim->SetKeyFrame(0.0f, pointA);
	valAnim->SetKeyFrame(restTime, pointA);
	valAnim->SetKeyFrame(restTime + speed, pointB);
	valAnim->SetKeyFrame((restTime * 2.0f) + speed, pointB);
	valAnim->SetKeyFrame((restTime + speed) * 2.0f, pointA);
	node_->SetAttributeAnimation("Position", valAnim, WM_LOOP, 1.0f);

	if (!node_->HasComponent<RigidBody>())
	{
		RigidBody* body = node_->CreateComponent<RigidBody>();
		body->SetMass(0);
		body->SetUseGravity(false);
		body->SetCollisionLayer(3);
		body->SetCollisionEventMode(COLLISION_ALWAYS);
		body->SetFriction(0.0f);
		body->SetAngularFactor(Vector3::ZERO);
		body->SetLinearFactor(Vector3::ZERO);
	}

	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Lift, OnCollision));
}

void Lift::FixedUpdate(float timeStep)
{
	if (activeRadius > 0.0f)
	{
		if ((game->playerNode->GetWorldPosition() - node_->GetWorldPosition()).Length() < activeRadius && timer <= 0.0f)
		{
			node_->SetAttributeAnimationSpeed("Position", 1.0f);
		}
		else
		{
			node_->SetAttributeAnimationSpeed("Position", 0.0f);
		}
	}
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
		VectorBuffer contacts = eventData["Contacts"].GetBuffer();
		while (!contacts.IsEof())
		{
			Vector3 position = contacts.ReadVector3();
			Vector3 normal = contacts.ReadVector3();
			float distance = contacts.ReadFloat();
			float impulse = contacts.ReadFloat();
			//if (fabs(normal.y_) >= 0.42f && position.y_ < other->GetWorldPosition().y_ + 0.4f)
			if (normal.y_ >= 0.75f)
			{
				//std::cout << "normal " << normal.y_ << std::endl;
				node_->SetAttributeAnimationSpeed("Position", 0.0f);
				timer = 0.5f;
				return;
			}
		}
	}
}

Lift::~Lift()
{
}
