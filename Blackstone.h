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

class Blackstone : public Projectile
{
	URHO3D_OBJECT(Blackstone, LogicComponent);
public:
	Blackstone(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~Blackstone();

	static Node* MakeBlackstone(Scene* sc, Vector3 position, Vector3 impulse, Node* owner);
protected:
	virtual void OnHit(Node* n);
	virtual void Move(const float timeStep);
	void ChangeState(const int newState);
	void OnPhysicsCollision(StringHash eventType, VariantMap& eventData);

	SharedPtr<ParticleEmitter> emitter;
	SharedPtr<RigidBody> body;
	WeakPtr<Node> target;
	Vector3 moveDir;
	float stateTimer;
	int state;
};

