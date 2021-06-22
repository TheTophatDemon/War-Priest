#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/ParticleEmitter.h>

#include "Enemy.h"
#include "SoundSounder.h"
#include "BlackHole.h"

using namespace Urho3D;

class Gameplay;
class KillerKube : public LogicComponent
{
	URHO3D_OBJECT(KillerKube, LogicComponent);
public:
	KillerKube(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	static void RegisterObject(Context* context);
	~KillerKube();
protected:
	void OnSettingsChange(StringHash eventType, VariantMap& eventData);
	void OnCollision(StringHash eventType, VariantMap& eventData);
	void OnAreaCollision(StringHash eventType, VariantMap& eventData);
	void OnCheatWindowEvent(StringHash eventType, VariantMap& eventData);
	void ChangeState(const int newState);
	void EnterState(const int newState);
	void LeaveState(const int oldState);

	SharedPtr<Gameplay> game;
	SharedPtr<ResourceCache> cache;
	SharedPtr<PhysicsWorld> physworld;
	SharedPtr<RigidBody> body;
	SharedPtr<Scene> scene;
	SharedPtr<Node> glowNode;
	SharedPtr<Node> blackHoleNode;
	SharedPtr<ValueAnimation> blackHoleAnimation;
	SharedPtr<ValueAnimation> spinSpeedAnimation;
	WeakPtr<SoundSounder> soundSource;
	WeakPtr<SoundSource> soundSource2D;
	WeakPtr<Node> soundNode;

	Vector3 direction;
	float deltaTime;
	float maxMoveSpeed;
	float moveSpeed;
	float spinSpeed;
	float distanceFromPlayer;
	float stateTimer;
	float targetHeight;
	float attackTimer;
	float shootTimer;
	float collideTime;
	int state;
	int lastState;
};
