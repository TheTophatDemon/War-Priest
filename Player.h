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
	void OnHurt(Node* source, int damage);
	~Player();
	static Vector3 orgShapeSize;
	static Vector3 orgShapePos;
	SharedPtr<Input> input;
	SharedPtr<Node> pivot;
	int health;
	int reviveCount = 0;
protected:
	void OnCollision(StringHash eventType, VariantMap& eventData);
	void OnAnimTrigger(StringHash eventType, VariantMap& eventData);
	void PostUpdate(StringHash eventType, VariantMap& eventData);
	void HandleCamera();
	void HandleShadow(PhysicsRaycastResult result);
	void ChangeState(int newState);

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

	Quaternion newRotation;
	Vector3 newCameraPosition;
	
	int state;
	int hailTimer;
	float stateTimer;
	int hurtTimer;

	float moveX = 0.0f;
	float moveZ = 0.0f;
	float lastShield = 0.0f;
};

