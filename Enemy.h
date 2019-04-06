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

//TODO: Remove redundant "Dead State" polymorphism
class Enemy : public LogicComponent
{
	//URHO3D_OBJECT(Enemy, LogicComponent);
public:
	Enemy(Context* context);
	virtual void Start() override;
	virtual void DelayedStart() override;
	virtual void FixedUpdate(float timeStep) override;
	virtual void Revive();
	inline int GetState() { return state; }
	virtual ~Enemy();
	bool revived;
	bool active;
protected:
	static const int STATE_DEAD = 0;
	static const int STATE_WANDER = 1;
	static const int STATE_DROWN = 2;
	static const int STATE_IDLE = 3;

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
	void ChangeState(const int newState);
	void FaceTarget();
	void KeepOnGround();
	virtual void Execute() = 0;
	virtual void Dead();
	virtual void EnterState(const int newState);
	virtual void LeaveState(const int oldState);
	virtual void OnCollision(StringHash eventType, VariantMap& eventData);

	virtual void Wander(bool pause = false, float slopeIntolerance = 0.5f, float wallMargin = 2.0f);
	bool CheckWall(PhysicsRaycastResult& result, const float wallMargin = 2.0f);
	bool CheckCliff(const float slopeIntolerance = 0.5f);
	bool CheckLift();
	void TurnRandomly(const float min = 30.0f, const float max = 180.0f);
	void ReflectOffNormal(const Vector3 normal);

	Quaternion newRotation;
	
	int state;
	float distanceFromPlayer;
	float stateTimer;
	float deltaTime;
	float restTimer;
	float turnTimer;
	float turnFactor;
	bool walking;
};

