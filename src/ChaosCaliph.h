#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/ParticleEmitter.h>

#include "Enemy.h"

using namespace Urho3D;

class Actor;
class ChaosCaliph : public Enemy
{
	URHO3D_OBJECT(ChaosCaliph, LogicComponent);
public:
	ChaosCaliph(Context* context);
	static void RegisterObject(Context* context);
	virtual void DelayedStart() override;
	virtual void Revive() override;
	virtual void FixedUpdate(float timeStep) override;
	~ChaosCaliph();
protected:
	static const int STATE_SHOOT = 32;
	static const int STATE_SPIN = 33;
	static float SPIN_RANGE;

	virtual void Execute() override;
	virtual void Dead() override;
	virtual void EnterState(const int newState) override;
	virtual void LeaveState(const int oldState) override;
	virtual void OnCollision(StringHash eventType, VariantMap& eventData) override;
	void OnSettingsChange(StringHash eventType, VariantMap& eventData);

	SharedPtr<AnimatedModel> animModel;
	SharedPtr<Material> glowyMaterial;
	SharedPtr<Material> boringMaterial;
	SharedPtr<ParticleEmitter> emitter;
	WeakPtr<Node> projectile;
	WeakPtr<Node> sparkChild;
	float originalMaxSpeed;
	int lastState;
	bool shot;
};
