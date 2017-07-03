#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Graphics/Material.h>

using namespace Urho3D;

class Launchpad : public LogicComponent
{
	URHO3D_OBJECT(Launchpad, LogicComponent);
public:
	Launchpad(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~Launchpad();

	float launchForce;
private:
	void OnCollision(StringHash eventType, VariantMap& eventData);
	
	SharedPtr<PhysicsWorld> physworld;
	SharedPtr<Scene> scene;
	SharedPtr<ResourceCache> cache;

	SharedPtr<SoundSource3D> soundSource;
	SharedPtr<Node> topNode;
	SharedPtr<Node> midNode;
	SharedPtr<Node> baseNode;

	SharedPtr<Material> material;
	SharedPtr<ValueAnimation> valAnim;

	float elapsedTime = 0.0f;
	float turnSpeedModifier = 0.0f;
};

