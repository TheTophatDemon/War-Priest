#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>

using namespace Urho3D;

class Statue : public LogicComponent
{
	URHO3D_OBJECT(Statue, LogicComponent);
public:
	Statue(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	void Damage(const int amount, const bool silent = false);
	int GetHealth();
	~Statue();
protected:
	void OnCollisionEnter(StringHash eventType, VariantMap& eventData);
	void OnProjectileHit(StringHash eventType, VariantMap& eventData);
	SharedPtr<Scene> scene;
	SharedPtr<PhysicsWorld> physworld;
	SharedPtr<ResourceCache> cache;
	SharedPtr<RigidBody> body;
	Vector3 originalPosition;
	float shakeTimer;
	float radius;
	int health;
};

