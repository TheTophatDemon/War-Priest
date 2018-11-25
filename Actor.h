#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/AngelScript/APITemplates.h>
#include "Lift.h"

using namespace Urho3D;

class Actor : public LogicComponent
{
	URHO3D_OBJECT(Actor, LogicComponent);
public:
	Actor(Context* context);
	static void RegisterObject(Context* context);
	
	virtual void Start();
	void SetInputFPS(bool fw, bool bk, bool lf, bool rg);
	void SetInputVec(const Vector3 mov);
	void Move(float timeStep);
	void Jump();
	void KnockBack(float amount, Quaternion direction);
	~Actor();

	PhysicsRaycastResult downCast;
	Vector3 input;

	float acceleration;
	float maxSpeed;
	float friction;
	float fallSpeed;
	float maxFall;
	float jumpStrength;
	float fall;
	bool onGround;
	bool sloping;
	bool gravity;
protected:
	void OnCollision(StringHash eventType, VariantMap& eventData);

	SharedPtr<PhysicsWorld> physworld;
	SharedPtr<Scene> scene;
	WeakPtr<RigidBody> body;
	SharedPtr<CollisionShape> shape;
	
	Vector3 rawMovement;
	Vector3 knockBackMovement;
	float deltaTime;
	float slopeSteepness;
};

