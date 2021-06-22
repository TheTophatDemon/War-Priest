#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>

using namespace Urho3D;

class Gameplay;
class GravityPlate : public LogicComponent
{
	friend class Gameplay;
	URHO3D_OBJECT(GravityPlate, LogicComponent);
public:
	GravityPlate(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start() override;
	virtual void FixedUpdate(float timeStep) override;
	~GravityPlate();
protected:
	WeakPtr<Gameplay> game;
	WeakPtr<RigidBody> body;
	Quaternion rotationDirection;
	Vector3 rotationAxis;
	float rotationForce;
};

