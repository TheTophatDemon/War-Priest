#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>

using namespace Urho3D;

class Actor;
class Gameplay;
class NPC : public LogicComponent
{
public:
	URHO3D_OBJECT(NPC, LogicComponent);
public:
	NPC(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	void ChangeState(int newState, int timer);
	~NPC();
protected:
	void MakeRagdoll();
	void HandleTurn();
	void Die();
	void OnCollision(StringHash eventType, VariantMap& eventData);

	SharedPtr<Node> modelNode;
	WeakPtr<Sound> voice;
	SharedPtr<SoundSource3D> soundSource;
	SharedPtr<Actor> actor;
	SharedPtr<RigidBody> body;
	SharedPtr<AnimatedModel> animatedModel;
	SharedPtr<AnimationController> animController;
	WeakPtr<Gameplay> game;
	WeakPtr<ResourceCache> cache;

	String resourcePath;
	float turn = 0.0f;
	int turnTimer = 0;
	int modelIndex = 0;
	int state = 0;
	int stateTimer = 0;
};

