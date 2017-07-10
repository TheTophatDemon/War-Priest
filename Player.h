#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D\Graphics/Camera.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/PhysicsUtils.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Audio/SoundSource.h>
#include "SoundSounder.h"
#include "Enemy.h"

using namespace Urho3D;

class Gameplay;
class Actor;
class Player : public LogicComponent
{
	URHO3D_OBJECT(Player, LogicComponent);
public:
	Player(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~Player();
	
	int state;
	int health;
	int reviveCount = 0;
protected:
	void OnCollision(StringHash eventType, VariantMap& eventData);
	void OnProjectileHit(StringHash eventType, VariantMap& eventData);
	void Hurt(Node* source, int damage);
	void HandleCamera();
	void HandleShadow();
	void ChangeState(int newState);
	void EnterState(int newState);
	void LeaveState(int oldState);
	void ST_Default(float timeStep);
	void ST_Revive(float timeStep);
	void ST_Slide(float timeStep);
	void ST_Win(float timeStep);
	void FindNearestCorpse();

	SharedPtr<Scene> scene;
	SharedPtr<ResourceCache> cache;
	WeakPtr<Gameplay> game;
	WeakPtr<PhysicsWorld> physworld;
	SharedPtr<Node> cameraNode;
	SharedPtr<Camera> camera;
	SharedPtr<Actor> actor;
	SharedPtr<RigidBody> body;
	SharedPtr<Node> modelNode;
	SharedPtr<AnimationController> animController;
	SharedPtr<Node> dropShadow;
	SharedPtr<RigidBody> groundDetector;
	SharedPtr<ParticleEmitter> bloodEmitter;
	SharedPtr<CollisionShape> shape;
	SharedPtr<Input> input;
	SharedPtr<Node> pivot;
	SharedPtr<SoundSounder> soundSource;
	WeakPtr<Enemy> nearestCorpse; //Weakptr instead of Sharedptr avoids big-ass memory leak.

	Quaternion newRotation;
	Vector3 slideDirection;
	Vector3 optimalCamPos;
	static Vector3 cameraOffset;
	
	int hailTimer;
	int hurtTimer;
	float stateTimer;
	float cameraPitch;
	float moveX = 0.0f;
	float moveZ = 0.0f;

	bool forwardKey = false;
	bool backwardKey = false;
	bool rightKey = false;
	bool leftKey = false;
	bool jumpKey = false;
	bool slideKey = false;
	bool reviveKey = false;
};

