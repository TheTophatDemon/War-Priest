#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>

using namespace Urho3D;

class Actor;
class NPC : public LogicComponent
{
public:
	URHO3D_OBJECT(NPC, LogicComponent);
public:
	NPC(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~NPC();
protected:
	void OnCollision(StringHash eventType, VariantMap& eventData);
	SharedPtr<Node> modelNode;
	SharedPtr<Actor> actor;
	SharedPtr<AnimatedModel> animatedModel;
	SharedPtr<AnimationController> animController;
	String resourcePath;
	int modelIndex;
};

