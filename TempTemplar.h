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
class TempShield;
class TempTemplar : public Enemy 
{
	URHO3D_OBJECT(TempTemplar, LogicComponent);
public:
	TempTemplar(Context* context);
	static void RegisterObject(Context* context);
	virtual void Revive() override;
	virtual void DelayedStart() override;
	~TempTemplar();
protected:
	static const int STATE_ATTACK = 32;
	static float MELEE_RANGE;
	static float MELEE_DAMAGE;

	void OnSettingsChange(StringHash eventType, VariantMap& eventData);
	void OnAnimTrigger(StringHash eventType, VariantMap& eventData);
	virtual void Execute() override;
	virtual void Dead() override;
	virtual void EnterState(const int newState) override;
	virtual void LeaveState(const int oldState) override;
	SharedPtr<Node> shieldNode;
	SharedPtr<TempShield> shieldComponent;
	bool attacked = false;
};

