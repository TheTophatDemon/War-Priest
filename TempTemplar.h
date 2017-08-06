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
class TempTemplar : public Enemy 
{
	URHO3D_OBJECT(TempTemplar, LogicComponent);
public:
	TempTemplar(Context* context);
	static void RegisterObject(Context* context);
	virtual void Revive();
	virtual void DelayedStart();
	~TempTemplar();
protected:
	void OnShieldCollision(StringHash eventType, VariantMap& eventData);
	virtual void Execute();
	virtual void Dead();
	virtual void EnterState(const int newState);
	virtual void LeaveState(const int oldState);
	WeakPtr<Node> shield;
	WeakPtr<StaticModel> shieldModel;
	WeakPtr<Node> subShield;
};

