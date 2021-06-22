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
	virtual void Start() override;
	~Projectile();

	WeakPtr<Node> owner;

	static StringHash E_PROJECTILEHIT;
	static StringHash P_PERPETRATOR;
	static StringHash P_VICTIM;
	static StringHash P_DAMAGE;
protected:
	virtual void Die() = 0;
	void PreUpdate(float timeStep);
	void PostUpdate(float timeStep);
	void DoDamage(Node* victim, const int damage);
	void ForceFieldResponse(Node* otherNode, const float turnSpeed);

	SharedPtr<Gameplay> game;
	SharedPtr<ResourceCache> cache;
	SharedPtr<PhysicsWorld> physworld;
	SharedPtr<Scene> scene;
	SharedPtr<RigidBody> body;

	float lifeTimer;
	float deltaTime;
	bool hit;
	//The node shouldn't be removed in the middle of an update; use this instead.
	bool killMe;
};

