#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/ParticleEmitter.h>

#include "Projectile.h"

using namespace Urho3D;

class Missile : public Projectile
{
	URHO3D_OBJECT(Missile, LogicComponent);
public:
	Missile(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~Missile();

	static Node* MakeMissile(Scene* sc, Vector3 position, Quaternion rotation, Node* owner, Node* target);
protected:
	virtual void OnCollision(StringHash eventType, VariantMap& eventData);
	virtual void Move(const float timeStep);
	SharedPtr<Node> emitterNode;
	SharedPtr<ParticleEmitter> emitter;
	WeakPtr<Node> target;
	Vector3 targetOffset; //Not neccessarily directly on the target. That's too boring.
	int state;
	float deltaTime;
};

