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

class BouncyFireball : public Projectile
{
	URHO3D_OBJECT(BouncyFireball, LogicComponent);
public:
	BouncyFireball(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~BouncyFireball();

	static Node* MakeBouncyFireball(Scene* sc, Vector3 position, Quaternion rotation, Node* owner);
protected:
	virtual void OnHit(PhysicsRaycastResult result);
	virtual void Move(const float timeStep);
	SharedPtr<ParticleEmitter> emitter;
};

