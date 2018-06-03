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
#include "WeakChild.h"

using namespace Urho3D;

#define LEVELMASK 2
#define GROUNDMARGIN 0.51f

Actor::Actor(Context* context) : LogicComponent(context)
{
	acceleration = 150.0f;
	maxspeed = 15.0f;
	friction = 0.85f;
	fallspeed = 50.0f;
	maxfall = 30.0f;
	jumpStrength = 18.0f;

	onGround = false;
	slopeSteepness = 0.0f;
	knockBack = 0.0f;
	deltaTime = 0.0f;
	gravity = true;

	forward = 0.0f;
	strafe = 0.0f;

	rawMovement = Vector3::ZERO;
	finalMovement = Vector3::ZERO;
}

void Actor::RegisterObject(Context* context)
{
	context->RegisterFactory<Actor>();
	URHO3D_ATTRIBUTE("Acceleration", float, acceleration, 150.0f, AM_DEFAULT);
	URHO3D_ATTRIBUTE("Max Speed", float, maxspeed, 15.0f, AM_DEFAULT);
	URHO3D_ATTRIBUTE("Friction", float, friction, 0.85f, AM_DEFAULT);
	URHO3D_ATTRIBUTE("Fall Speed", float, fallspeed, 50.0f, AM_DEFAULT);
	URHO3D_ATTRIBUTE("Max Fall", float, maxfall, 30.0f, AM_DEFAULT);
	URHO3D_ATTRIBUTE("Jump Strength", float, jumpStrength, 18.0f, AM_DEFAULT);
	URHO3D_ATTRIBUTE("On Ground", bool, onGround, false, AM_DEFAULT);
	URHO3D_ATTRIBUTE("Gravity Enabled", bool, gravity, true, AM_DEFAULT);
}

void Actor::ApplyAttributes()
{
	acceleration = node_->GetAttribute("Acceleration").GetFloat();
	maxspeed = node_->GetAttribute("Max Speed").GetFloat();
	friction = node_->GetAttribute("Friction").GetFloat();
	fallspeed = node_->GetAttribute("Fall Speed").GetFloat();
	maxfall = node_->GetAttribute("Max Fall").GetFloat();
	jumpStrength = node_->GetAttribute("Jump Strength").GetFloat();
	gravity = node_->GetAttribute("Gravity Enabled").GetBool();
}

void Actor::Start()
{
	body = node_->GetComponent<RigidBody>();
	scene = node_->GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	shape = node_->GetComponent<CollisionShape>();

	//SubscribeToEvent(GetNode(), E_NODECOLLISIONSTART, URHO3D_HANDLER(Actor, OnCollisionStart));
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Actor, OnCollision));
	//SubscribeToEvent(GetNode(), E_NODECOLLISIONEND, URHO3D_HANDLER(Actor, OnCollisionEnd));
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
	strafe = 0.0f; forward = 0.0f;
	rawMovement = Vector3(xm, 0.0f, zm);
}
void Actor::SetMovement(Vector3 mv)
{
	strafe = 0.0f; forward = 0.0f;
	rawMovement = mv;
}

void Actor::Jump()
{
	if (onGround)
	{
		fall = jumpStrength;
		onGround = false;
	}
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
				slopeFall = (-1 / (slopeSteepness * 0.64f) + 1) * rawMovement.Length();
				const Vector3 slopeNormalXZ = Vector3(downCast.normal_.x_, 0.0f, downCast.normal_.z_);
				const float dot = slopeNormalXZ.DotProduct(rawMovement);
				if (dot < -0.75f && slopeSteepness >= 0.75f)
				{
					slopeFall = fabs(slopeFall) * 0.34f;
				}
				sloping = true;
			}
		}
	}
	else if (gravity)
	{
		fall -= fallspeed * deltaTime;
		if (fall < -maxfall) fall = -maxfall;
	}
	else
	{
		fall = 0.0f;
	}

	//Manual ceiling collision check: Because physics are RETARDED!
	if (fall > 0.1f)
	{
		PhysicsRaycastResult upCast;
		physworld->RaycastSingle(upCast, Ray(node_->GetWorldPosition() + Vector3(0.0f, shape->GetSize().y_, 0.0f), Vector3::UP), 10.0f, 2);
		if (upCast.body_)
		{
			if (upCast.distance_ < shape->GetSize().y_ / 4.0f)
			{
				fall = 0.0f;
			}
		}
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

	Vector3 transformedMovement = Vector3(rawMovement.x_, fall + slopeFall, rawMovement.z_);

	finalMovement = ((transformedMovement + (knockBackDirection * Vector3::FORWARD * knockBack)) * deltaTime * 50.0f);
	body->SetLinearVelocity(finalMovement);
	rawMovement = Vector3::ZERO;

	slopeSteepness = 1.0f;
	GetSlope();

	if (downCast.body_)
	{
		if (downCast.distance_ <= 0.6f && downCast.body_->GetNode()->HasTag("lift"))
		{
			//Sending a fake collision event to fix a weird bug where lifts occasionally forget that things are standing on them
			VariantMap eventData = VariantMap();
			eventData.Insert(Pair<StringHash, Variant>(NodeCollision::P_OTHERBODY, Variant(body)));
			eventData.Insert(Pair<StringHash, Variant>(NodeCollision::P_OTHERNODE, Variant(node_)));
			eventData.Insert(Pair<StringHash, Variant>(NodeCollision::P_TRIGGER, false));
			eventData.Insert(Pair<StringHash, Variant>(NodeCollision::P_BODY, Variant(downCast.body_)));

			VectorBuffer contacts = VectorBuffer();
			contacts.WriteVector3(Vector3::ZERO); //Position. Ignore.
			contacts.WriteVector3(Vector3(0.0f, -666.0f, 0.0f)); //Normal. Must be negative on Y to be recognized as "on top"
			contacts.WriteFloat(downCast.distance_); //Distance
			contacts.WriteFloat(0.0f); //Impulse. Ignored.
			eventData.Insert(Pair<StringHash, Variant>(NodeCollision::P_CONTACTS, contacts));

			downCast.body_->GetNode()->SendEvent(E_NODECOLLISION, eventData);
		}
	}

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
	physworld->RaycastSingle(downCast, Ray(node_->GetWorldPosition() + Vector3(0.0f, 0.5f, 0.0f), Vector3::DOWN), 500.0f, LEVELMASK);
	if (downCast.body_)
	{
		slopeSteepness = downCast.normal_.y_;
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
			if (fabs(normal.y_) >= 0.75f)
			{
				if (position.y_ <= node_->GetWorldPosition().y_ + 0.5f)
				{
					onGround = true;
				}
				else if (position.y_ >= node_->GetWorldPosition().y_ + 3.0f)
				{
					if (fall > 0.0f) fall = 0.0f;
				}
			}
		}
	}
	else if (otherBody->GetCollisionLayer() & 16)
	{
		if (other->HasComponent<WeakChild>()) //Check if the shield is one that this node owns
		{
			WeakChild* wc = other->GetComponent<WeakChild>();
			if (wc->parent.Get() == node_) goto skip;
		}
		if (other->HasTag("tempshield"))
		{
			Vector3 diff = node_->GetWorldPosition() - other->GetWorldPosition();
			diff.y_ = 0.0f;
			Quaternion direction = Quaternion();
			direction.FromLookRotation(diff.Normalized(), Vector3::UP);
			KnockBack(10.0f + (12.0f / diff.LengthSquared()), direction);
		}
		else if (other->HasTag("blackhole"))
		{
			Vector3 diff = other->GetWorldPosition() - node_->GetWorldPosition();
			Quaternion direction = Quaternion();
			direction.FromLookRotation(diff.Normalized(), Vector3::UP);
			KnockBack(15.0f + (35.0f / diff.LengthSquared()), direction);
		}
	skip: {} //I am a PROFESSIONAL C++ DEVELOPER
	}
}

Actor::~Actor()
{
}
