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
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/Sound.h>
#include "SoundSounder.h"

using namespace Urho3D;

class Gameplay;
class Actor;
class Projectile;
class Enemy : public LogicComponent
{
	URHO3D_OBJECT(Enemy, LogicComponent);
public:
	Enemy(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void DelayedStart();
	virtual void FixedUpdate(float timeStep);
	virtual void Revive();
	~Enemy();
	int state;
	bool revived = false;
protected:
	SharedPtr<Gameplay> game;
	SharedPtr<ResourceCache> cache;
	SharedPtr<PhysicsWorld> physworld;
	SharedPtr<Node> modelNode;
	SharedPtr<Actor> actor;
	SharedPtr<RigidBody> body;
	SharedPtr<Scene> scene;
	SharedPtr<CollisionShape> shape;
	Quaternion newRotation;
	SharedPtr<CollisionShape> oldShape;
	SharedPtr<Node> target;
	SharedPtr<SoundSounder> soundSource;
	SharedPtr<AnimationController> animController;

	float distanceFromPlayer;

	void EndFrameCheck(StringHash eventType, VariantMap& eventData);
	
	float stateTimer;

	virtual void Execute();
	void Wander();
	void Dead();
	bool CheckCliff();
	void ChangeState(const int newState);
	virtual void EnterState(const int newState);
	virtual void LeaveState(const int oldState);
	void FaceTarget();
	void KeepOnGround();

	float deltaTime;
	float turnAmount = 0.0f;
	float turnTimer = 0.0f;
	bool walking = false;
};

