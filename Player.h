#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D\Graphics/Camera.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/ParticleEmitter.h>

using namespace Urho3D;

class Gameplay;
class Actor;
class Player : public LogicComponent
{
	URHO3D_OBJECT(Player, LogicComponent);
public:
	SharedPtr<Input> input;
	SharedPtr<Node> pivot;

	Player(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~Player();
protected:
	void OnCollision(StringHash eventType, VariantMap& eventData);
	void OnAnimTrigger(StringHash eventType, VariantMap& eventData);
	void FireWeapon();
	void SummonBoulder();

	SharedPtr<Scene> scene;
	SharedPtr<ResourceCache> cache;
	WeakPtr<Gameplay> game;
	WeakPtr<PhysicsWorld> physworld;
	SharedPtr<Node> cameraNode;
	SharedPtr<Camera> camera;
	SharedPtr<Actor> actor;
	SharedPtr<RigidBody> body;
	SharedPtr<Node> modelNode;

	SharedPtr<Node> shrapnel;
	SharedPtr<ParticleEmitter> shrapnelEmitter;

	Quaternion newRotation;
	float cameraPitch;
};

