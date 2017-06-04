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
#include <Urho3D/Container/Vector.h>

#include "Enemy.h"

using namespace Urho3D;

class Actor;
class Gameplay;
class PostalPope : Enemy
{
	URHO3D_OBJECT(PostalPope, LogicComponent);
public:
	PostalPope(Context* context);
	static void RegisterObject(Context* context);
	virtual void Revive();
	virtual void DelayedStart();
	~PostalPope();
protected:
	virtual void Execute();
	virtual void Dead();
	virtual void EnterState(int newState);
	virtual void LeaveState(int oldState);
	Vector<Node*> debris;
	SharedPtr<Node> spinner;
	SharedPtr<SoundSource> weeoo;
};

