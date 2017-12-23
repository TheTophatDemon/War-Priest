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
	//URHO3D_OBJECT(Enemy, LogicComponent);
public:
	Enemy(Context* context);
	virtual void Start();
	virtual void DelayedStart();
	virtual void FixedUpdate(float timeStep);
	virtual void Revive();
	virtual ~Enemy();
	bool revived = false;
	bool active = false;
protected:
	SharedPtr<Gameplay> game;
	SharedPtr<ResourceCache> cache;
	SharedPtr<PhysicsWorld> physworld;
	SharedPtr<Node> modelNode;
	SharedPtr<Actor> actor;
	SharedPtr<RigidBody> body;
	SharedPtr<Scene> scene;
	SharedPtr<CollisionShape> shape;
	SharedPtr<CollisionShape> oldShape;
	SharedPtr<Node> target;
	SharedPtr<SoundSounder> soundSource;
	SharedPtr<AnimationController> animController;

	void EndFrameCheck(StringHash eventType, VariantMap& eventData);
	virtual void Execute();
	void Wander(const bool avoidSlopes = false, const bool pause = false, const float wallMargin = 2.0f);
	void Dead();
	bool CheckCliff(const bool avoidSlopes = false);
	void ChangeState(const int newState);
	virtual void EnterState(const int newState);
	virtual void LeaveState(const int oldState);
	void FaceTarget();
	void KeepOnGround();

	Quaternion newRotation;

	float distanceFromPlayer;
	float stateTimer;
	int state;
	float deltaTime;
	float turnAmount = 0.0f;
	float turnTimer = 0.0f;
	float talkTimer = 0.0f;
	bool walking = false;
};

