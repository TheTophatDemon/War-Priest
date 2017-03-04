#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D\Graphics/Camera.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Graphics/ParticleEmitter.h>

using namespace Urho3D;

class Gameplay;
class Actor;
class Player : public LogicComponent
{
	URHO3D_OBJECT(Player, LogicComponent);
public:
	SharedPtr<Input> input;

	Player(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~Player();
protected:
	void OnCollision(StringHash eventType, VariantMap& eventData);
	void OnAnimTrigger(StringHash eventType, VariantMap& eventData);

	SharedPtr<Node> arms;
	SharedPtr<Node> cameraNode;
	SharedPtr<Scene> scene;
	SharedPtr<ParticleEmitter> leftMuzzleFlash;
	SharedPtr<ParticleEmitter> rightMuzzleFlash;
	SharedPtr<Actor> actor;
	WeakPtr<Gameplay> game;
	WeakPtr<RigidBody> body;
	WeakPtr<PhysicsWorld> physworld;

	float cameraPitch = 0.0f;
};

