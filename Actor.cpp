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
#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/AngelScript/ScriptAPI.h>
#include <Urho3D/AngelScript/APITemplates.h>
#include <iostream>
#include "WeakChild.h"
#include "TempShield.h"

using namespace Urho3D;

#define LEVELMASK 2

Actor::Actor(Context* context) : LogicComponent(context),
	acceleration(175.0f),
	maxSpeed(15.0f),
	friction(60.0f),
	fallSpeed(50.0f),
	maxFall(30.0f),
	jumpStrength(18.0f),
	onGround(false),
	slopeSteepness(0.0f),
	deltaTime(0.0f),
	gravity(true),
	knockBackMovement(Vector3::ZERO),
	input(Vector3::ZERO),
	rawMovement(Vector3::ZERO)
{
}

void Actor::RegisterObject(Context* context)
{
	context->RegisterFactory<Actor>("War Priest");

	//Make it accessible to scripting api
	asIScriptEngine* scrEngine = context->GetSubsystem<Script>()->GetScriptEngine();
	RegisterComponent<Actor>(scrEngine, "Actor");
	scrEngine->RegisterObjectMethod("Actor", "void Move(float timeStep)", asMETHOD(Actor, Move), asCALL_THISCALL);
	scrEngine->RegisterObjectMethod("Actor", "void SetInputFPS(bool fw, bool bk, bool lf, bool rg)", asMETHOD(Actor, SetInputFPS), asCALL_THISCALL);
	scrEngine->RegisterObjectMethod("Actor", "void SetInputVec(const Vector3 mov)", asMETHOD(Actor, SetInputVec), asCALL_THISCALL);
	scrEngine->RegisterObjectMethod("Actor", "void Jump()", asMETHOD(Actor, Jump), asCALL_THISCALL);
	scrEngine->RegisterObjectMethod("Actor", "void KnockBack(float amount, Quaternion direction)", asMETHOD(Actor, KnockBack), asCALL_THISCALL);
	scrEngine->RegisterObjectProperty("Actor", "Vector3 input", offsetof(Actor, input));
	scrEngine->RegisterObjectProperty("Actor", "PhysicsRaycastResult downCast", offsetof(Actor, downCast));
	scrEngine->RegisterObjectProperty("Actor", "float acceleration", offsetof(Actor, acceleration));
	scrEngine->RegisterObjectProperty("Actor", "float maxSpeed", offsetof(Actor, maxSpeed));
	scrEngine->RegisterObjectProperty("Actor", "float friction", offsetof(Actor, friction));
	scrEngine->RegisterObjectProperty("Actor", "float fallSpeed", offsetof(Actor, fallSpeed));
	scrEngine->RegisterObjectProperty("Actor", "float maxFall", offsetof(Actor, maxFall));
	scrEngine->RegisterObjectProperty("Actor", "float jumpStrength", offsetof(Actor, jumpStrength));
	scrEngine->RegisterObjectProperty("Actor", "float fall", offsetof(Actor, fall));
	scrEngine->RegisterObjectProperty("Actor", "bool onGround", offsetof(Actor, onGround));
	scrEngine->RegisterObjectProperty("Actor", "bool sloping", offsetof(Actor, sloping));
	scrEngine->RegisterObjectProperty("Actor", "bool gravity", offsetof(Actor, gravity));
}

void Actor::Start()
{
	SetUpdateEventMask(0);

	body = node_->GetComponent<RigidBody>();
	scene = node_->GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	shape = node_->GetComponent<CollisionShape>();

	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Actor, OnCollision));
}

void Actor::SetInputFPS(bool fw, bool bk, bool lf, bool rg)
{
	if (fw) 
	{
		input.z_ = 1.0f;
	}
	else if (bk) 
	{
		input.z_ = -1.0f;
	}
	else
	{
		input.z_ = 0.0f;
	}
	if (rg)
	{
		input.x_ = 1.0f;
	}
	else if (lf)
	{
		input.x_ = -1.0f;
	}
	else
	{
		input.x_ = 0.0f;
	}
	input.y_ = 0.0f;
	input = node_->GetWorldRotation() * input;
	input.Normalize();
}

void Actor::SetInputVec(const Vector3 mov)
{
	input.x_ = mov.x_;
	input.y_ = mov.y_;
	input.z_ = mov.z_;
	input.Normalize();
}

void Actor::Jump()
{
	if (gravity) 
	{
		if (onGround)
		{
			fall = jumpStrength;
			onGround = false;
		}
	}
	else
	{
		rawMovement.y_ += jumpStrength;
	}
}

void Actor::Move(float timeStep)
{
	deltaTime = timeStep;

	//Horizontal plane movement
	rawMovement += input * acceleration * deltaTime;
	const float rawMagnitude = rawMovement.Length();
	//Friction
	if (input == Vector3::ZERO)
	{
		rawMovement = (rawMovement / rawMagnitude) * Max(0.0f, rawMagnitude - friction * deltaTime);
	}
	//Clamping
	if (rawMagnitude > maxSpeed)
	{
		rawMovement = (rawMovement / rawMagnitude) * maxSpeed;
	}
	else if (rawMagnitude < 0.1f)
	{
		rawMovement = Vector3::ZERO;
	}

	//Falling logic
	sloping = false;
	float slopeFall = 0.0f;

	if (gravity) 
	{
		if (onGround)
		{
			if (fall <= 0.0f)
			{
				fall = -0.1f;
				if (slopeSteepness != 1.0f && slopeSteepness >= 0.42f)
				{
					const Vector3 rawMovementXZ = Vector3(rawMovement.x_, 0.0f, rawMovement.z_);
					const Vector3 slopeNormalXZ = Vector3(downCast.normal_.x_, 0.0f, downCast.normal_.z_);
					slopeFall = (-1 / (slopeSteepness * 0.64f) + 1) * rawMovementXZ.Length();
					const float dot = slopeNormalXZ.DotProduct(rawMovementXZ);
					if (dot < -0.75f && slopeSteepness >= 0.75f)
					{
						slopeFall = fabs(slopeFall) * 0.34f;
					}
					sloping = true;
				}
			}
		}
		else
		{
			fall -= fallSpeed * deltaTime;
			if (fall < -maxFall) fall = -maxFall;
		}
	}
	else
	{
		fall = 0.0f;
	}

	/*
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
	}*/

	//Apply movements
	const float knockBackMagnitude = knockBackMovement.Length();
	if (knockBackMagnitude > 0.01f)
	{
		knockBackMovement = (knockBackMovement / knockBackMagnitude) * Max(0.0f, knockBackMagnitude - friction * deltaTime);
	}
	else
	{
		knockBackMovement = Vector3::ZERO;
	}
	const Vector3 combinedMovement = Vector3(rawMovement.x_, rawMovement.y_ + fall + slopeFall, rawMovement.z_);
	body->SetLinearVelocity((combinedMovement + knockBackMovement) * deltaTime * 50.0f);

	//Raycast downward to get slope normal
	physworld->RaycastSingle(downCast, Ray(node_->GetWorldPosition() + Vector3(0.0f, 0.5f, 0.0f), Vector3::DOWN), 10.0f, LEVELMASK);
	if (downCast.body_)
	{
		slopeSteepness = downCast.normal_.y_;
	}
	else
	{
		slopeSteepness = 1.0f;
	}

	//Sending a fake collision event to fix a weird bug where lifts occasionally forget that things are standing on them
	if (downCast.body_)
	{
		if (downCast.distance_ <= 0.6f && downCast.body_->GetNode()->HasTag("lift"))
		{	
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
	knockBackMovement = (direction * Vector3::FORWARD) * amount;
}

void Actor::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();

	if (otherBody->GetCollisionLayer() & LEVELMASK) //Get ground information
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
				const float yDiff = position.y_ - body->GetPosition().y_;
				if (fall > 0.0f && yDiff > shape->GetSize().y_ / 2.0f)
				{
					fall = 0.0f;
				}
			}
		}
	}
	else if (otherBody->GetCollisionLayer() & 16) //React to shields and black holes
	{
		if (other->HasTag("tempshield"))
		{
			if (other->GetComponent<TempShield>()->owner.Get() != node_) 
			{
				Vector3 diff = node_->GetWorldPosition() - other->GetWorldPosition();
				diff.y_ = 0.0f;
				const float lensq = diff.LengthSquared();
				if (lensq != 0.0f) 
				{
					Quaternion direction = Quaternion();
					direction.FromLookRotation(diff.Normalized(), Vector3::UP);
					KnockBack(10.0f + (12.0f / lensq), direction);
				}
			}
		}
		else if (other->HasTag("blackhole"))
		{
			Vector3 diff = other->GetWorldPosition() - node_->GetWorldPosition();
			const float lensq = diff.LengthSquared();
			if (lensq != 0.0f) 
			{
				Quaternion direction = Quaternion();
				direction.FromLookRotation(diff.Normalized(), Vector3::UP);
				KnockBack(17.5f + (35.0f / lensq), direction);
			}
		}
	}
}

Actor::~Actor()
{
}
