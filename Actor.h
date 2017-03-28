#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>

using namespace Urho3D;

class Actor : public LogicComponent
{
public:
	Actor(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	void Move(bool fw, bool bk, bool rg, bool lf, bool jmp, float timeStep);
	~Actor();

	Vector3 movement;
	float acceleration = 0.0f;
	float maxspeed = 0.0f;
	float friction = 0.0f;
	float fallspeed = 0.0f;
	float maxfall = 0.0f;
	float jumpStrength = 0.0f;

	float forward = 0.0f;
	float strafe = 0.0f;
	float fall = 0.0f;
	bool onGround = false;
	float slopeSteepness;
protected:
	void GetSlope();
	void StairCheck();
	void OnCollision(StringHash eventType, VariantMap& eventData);
	virtual void ChangeState(int newState);
	SharedPtr<PhysicsWorld> physworld;
	SharedPtr<Scene> scene;
	WeakPtr<RigidBody> body;
	int aiState;
};

