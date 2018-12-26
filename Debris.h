#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>

using namespace Urho3D;

//Represents a Postal Pope's THROWN boulder
class Debris : public LogicComponent
{
	URHO3D_OBJECT(Debris, LogicComponent);
public:
	Debris(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	int damage;
	~Debris();
protected:
	void OnCollisionStart(StringHash eventType, VariantMap& eventData);
	void Die();

	SharedPtr<Scene> scene;
	SharedPtr<RigidBody> body;
	SharedPtr<PhysicsWorld> physworld;
	SharedPtr<ResourceCache> cache;
	SharedPtr<SoundSource3D> crashSource;
	WeakPtr<Node> smokeNode;

	float linearVelocity = 0.0f;
	float dieTimer = 0.0f;
};

