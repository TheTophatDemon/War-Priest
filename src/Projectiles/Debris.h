#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>

#include "Projectile.h"

using namespace Urho3D;

class Gameplay;
//Represents a Postal Pope's THROWN boulder
class Debris : public Projectile
{
	URHO3D_OBJECT(Debris, LogicComponent);
public:
	Debris(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start() override;
	virtual void FixedUpdate(float timeStep) override;
	
	int damage;
	~Debris();
protected:
	virtual void Die() override;
	void OnCollisionStart(StringHash eventType, VariantMap& eventData);
	void OnCollision(StringHash eventType, VariantMap& eventData);
	SharedPtr<SoundSource3D> crashSource;
	WeakPtr<Node> smokeNode;

	float linearVelocity;
	float dieTime;
};

