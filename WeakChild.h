#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Resource/ResourceCache.h>

using namespace Urho3D;

class WeakChild : public LogicComponent //This class is for nodes created for other nodes, that should be deleted alongside them, but shouldn't have to inherit position.
{
	URHO3D_OBJECT(WeakChild, LogicComponent);
public:
	WeakChild(Context* context);
	static void RegisterObject(Context* context);
	static void MakeWeakChild(Node* child, Node* parent);
	virtual void FixedUpdate(float timeStep);
	~WeakChild();
	WeakPtr<Node> parent;
};

