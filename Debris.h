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
	void OnCollision(StringHash eventType, VariantMap& eventData);
	void Die();
	SharedPtr<Scene> scene;
	SharedPtr<RigidBody> body;
	SharedPtr<PhysicsWorld> physworld;
	SharedPtr<ResourceCache> cache;
	SharedPtr<Node> glowNode;
};

