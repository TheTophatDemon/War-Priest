#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>

#include "Projectile.h"

class RobeLocksMissile : public Projectile
{
	URHO3D_OBJECT(RobeLocksMissile, LogicComponent);
public:
	RobeLocksMissile(Context* context);
	~RobeLocksMissile();
	static void RegisterObject(Context* context);
	virtual void Start() override;
	virtual void FixedUpdate(float timeStep) override;

	static Node* MakeRobeLocksMissile(Scene* scene, const Vector3 position, const Quaternion rotation, Node* owner);
protected:
	virtual void Die() override;
	void OnCollision(StringHash eventType, VariantMap& eventData);
	
	float speed;
};

