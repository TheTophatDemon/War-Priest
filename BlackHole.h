#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Graphics/Technique.h>

#include "KillerKube.h"

using namespace Urho3D;

class Gameplay;
class BlackHole : public LogicComponent
{
	friend class KillerKube;
	URHO3D_OBJECT(BlackHole, LogicComponent);
public:
	BlackHole(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start() override;
	virtual void FixedUpdate(float timeStep) override;
	static Node* MakeBlackHole(Scene* scene, const Vector3 position, Node* owner);
	~BlackHole();

	WeakPtr<Node> owner;
protected:
	void OnCollision(StringHash eventType, VariantMap& eventData);
	WeakPtr<Gameplay> game;
	WeakPtr<Pass> pass;
};

