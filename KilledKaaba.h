#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/ParticleEmitter.h>

#include "Enemy.h"

using namespace Urho3D;

class Gameplay;
class KilledKaaba : public LogicComponent
{
	URHO3D_OBJECT(KilledKaaba, LogicComponent);
public:
	KilledKaaba(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	static void RegisterObject(Context* context);
	~KilledKaaba();
protected:
	void OnSettingsChange(StringHash eventType, VariantMap& eventData);
	void OnCollision(StringHash eventType, VariantMap& eventData);
	void OnAreaCollision(StringHash eventType, VariantMap& eventData);

	SharedPtr<Gameplay> game;
	SharedPtr<ResourceCache> cache;
	SharedPtr<PhysicsWorld> physworld;
	SharedPtr<RigidBody> body;
	SharedPtr<Scene> scene;
	SharedPtr<Node> glowNode;
	SharedPtr<Node> target;
	void ChangeState(const int newState);
	void EnterState(const int newState);
	void LeaveState(const int oldState);
	
	Vector<SharedPtr<Node>> areas;

	Vector3 direction;
	float moveSpeed;
	float distanceFromPlayer;
	float stateTimer;
	float targetHeight;
	int state;
	int lastState;
};

