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
#include "Blackstone.h"

using namespace Urho3D;

class Actor;
class ChaosCaliph : public Enemy
{
	URHO3D_OBJECT(ChaosCaliph, LogicComponent);
public:
	ChaosCaliph(Context* context);
	static void RegisterObject(Context* context);
	virtual void DelayedStart();
	virtual void Revive();
	~ChaosCaliph();
protected:
	virtual void Execute();
	virtual void Dead();
	virtual void EnterState(const int newState);
	virtual void LeaveState(const int oldState);
	void OnCollision(StringHash eventType, VariantMap& eventData);

	SharedPtr<AnimatedModel> animModel;
	SharedPtr<Material> glowyMaterial;
	SharedPtr<Material> boringMaterial;
	SharedPtr<ParticleEmitter> emitter;
	WeakPtr<Node> projectile;
	WeakPtr<Node> sparkChild;
	float spinSpeed = 0.0f;
	int lastState;
	bool shot;
};

