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

class Fireball : public Projectile
{
	URHO3D_OBJECT(Fireball, LogicComponent);
public:
	Fireball(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start() override;
	virtual void FixedUpdate(float timeStep) override;
	~Fireball();

	static Node* MakeFireball(Scene* sc, Vector3 position, Quaternion rotation, Node* owner);
	static Node* MakeBlueFireball(Scene* sc, Vector3 position, Quaternion rotation, Node* owner);
	static Node* MakePaintball(Scene* sc, Vector3 position, Quaternion rotation, Node* owner, const float lifeSpan, const float speed);
protected:
	virtual void Die() override;
	void OnCollision(StringHash eventType, VariantMap& eventData);

	SharedPtr<ParticleEmitter> emitter;
	
	float deathTime;
	float speed;
	float lifeSpan;
	int damage;
};

