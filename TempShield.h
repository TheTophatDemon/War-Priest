#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
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
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~TempShield();
	WeakPtr<Node> owner;
protected:
	void OnCollision(StringHash eventType, VariantMap& eventData);
	WeakPtr<RigidBody> body;
	WeakPtr<StaticModel> model;
	WeakPtr<Node> subShield;
	float maxSize;
	bool formed;
};

