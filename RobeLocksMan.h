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
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/Sound.h>

#include "Enemy.h"
#include "SoundSounder.h"

enum class Weapon
{
	Launcher,
	Bomb,
	PaintGun
};

class RobeLocksMan : public Enemy
{
	URHO3D_OBJECT(RobeLocksMan, LogicComponent);
public:
	RobeLocksMan(Context* context);
	static void RegisterObject(Context* context);
	virtual void DelayedStart() override;
	virtual void Revive() override;
	~RobeLocksMan();
protected:
	virtual void Execute() override;
	virtual void Dead() override;
	virtual void EnterState(const int newState) override;
	virtual void LeaveState(const int oldState) override;

	SharedPtr<SoundSounder> soundSource;
	SharedPtr<Node> weaponNode;
	SharedPtr<StaticModel> weaponModel;
	Weapon currentWeapon;
	float shootTimer;
};

