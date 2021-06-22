#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Physics/RigidBody.h>

#include "TempTemplar.h"

using namespace Urho3D;

class TempShield : public LogicComponent
{
	friend class TempTemplar;
	URHO3D_OBJECT(TempShield, LogicComponent);
public:
	TempShield(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start() override;
	virtual void FixedUpdate(float timeStep) override;
	static Node* MakeTempShield(Scene* scene, const Vector3 position, Node* owner);
	~TempShield();

	WeakPtr<Node> owner;
protected:
	void OnCollision(StringHash eventType, VariantMap& eventData);
	WeakPtr<StaticModel> model;
	float maxSize;
	bool formed;
};

