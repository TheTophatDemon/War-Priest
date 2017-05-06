#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Graphics/ParticleEmitter.h>

using namespace Urho3D;

class Gameplay;
class Projectile : public LogicComponent
{
	URHO3D_OBJECT(Projectile, LogicComponent);
public:
	Projectile(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	void Destroy();
	~Projectile();
	Node* owner = nullptr;
	float radius;
	float speed;
	int damage;
	int projectileType;

	static Node* MakeProjectile(Scene* sc, String name, Vector3 position, Quaternion rotation, Node* owner);
protected:
	SharedPtr<Gameplay> game;
	SharedPtr<ResourceCache> cache;
	SharedPtr<PhysicsWorld> physworld;
	SharedPtr<Scene> scene;
	SharedPtr<ParticleEmitter> emitter;
	Vector3 movement;
	int timer;
	float lifeTimer;
	
	bool hit;
};

