#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D\Graphics/ParticleEmitter.h>

#include "Enemy.h"

using namespace Urho3D;

class Actor;
class Gameplay;
class PyroPastor : Enemy
{
	URHO3D_OBJECT(PyroPastor, LogicComponent);
public:
	PyroPastor(Context* context);
	static void RegisterObject(Context* context);
	virtual void Revive();
	virtual void DelayedStart();
	~PyroPastor();
protected:
	virtual void Execute();
	virtual void Dead();
	virtual void EnterState(int newState);
	virtual void LeaveState(int oldState);
	void FaceTarget();

	SharedPtr<AnimationController> animController;
	WeakPtr<Node> target;
	float turnAmount = 0.0f;
	float turnTimer = 0.0f;
	bool walking = false;
	bool shot = false;
};

