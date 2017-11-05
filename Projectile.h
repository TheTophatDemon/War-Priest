#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>

using namespace Urho3D;

class Gameplay;
class Projectile : public LogicComponent
{
public:
	Projectile(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~Projectile();

	WeakPtr<Node> owner;
	float radius;
	float speed;
	int damage;

	static StringHash E_PROJECTILEHIT;
protected:
	virtual void OnHit(Node* n);
	virtual void Move(const float timeStep) = 0;

	SharedPtr<Gameplay> game;
	SharedPtr<ResourceCache> cache;
	SharedPtr<PhysicsWorld> physworld;
	SharedPtr<Scene> scene;

	Vector3 movement;
	float deathTimer;
	float lifeTimer;
	float orgSpeed = 0.0f;
	bool hit;
};

