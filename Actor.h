#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>

using namespace Urho3D;

class Actor : public LogicComponent
{
public:
	Actor(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	void SetMovement(bool fw, bool bk, bool lf, bool rg);
	void SetMovement(float xm, float zm);
	void SetMovement(Vector3 mv);
	void Move(float timeStep);
	void Jump();
	void KnockBack(float amount, Quaternion direction);
	~Actor();

	Vector3 rawMovement;
	Vector3 finalMovement;
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
	bool sloping = false;
	float slopeSteepness;
protected:
	float deltaTime;
	void GetSlope();
	void StairCheck();
	void OnCollision(StringHash eventType, VariantMap& eventData);
	void PreStep(StringHash eventType, VariantMap& eventData);
	virtual void ChangeState(int newState);
	SharedPtr<PhysicsWorld> physworld;
	SharedPtr<Scene> scene;
	WeakPtr<RigidBody> body;
	SharedPtr<CollisionShape> shape;

	Quaternion knockBackDirection;
	float knockBack;
	int aiState;
	
};

