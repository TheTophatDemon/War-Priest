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
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/Sound.h>

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
	virtual void Revive() override;
	virtual void DelayedStart() override;
	~PyroPastor();
protected:
	static const int STATE_ATTACK = 32;

	virtual void Execute() override;
	virtual void Dead() override;
	virtual void EnterState(const int newState) override;
	virtual void LeaveState(const int oldState) override;
	
	bool shot = false;
};

