#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>

using namespace Urho3D;

class Gameplay;
class Cross : public LogicComponent
{
	URHO3D_OBJECT(Cross, LogicComponent);
public:
	Cross(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	~Cross();
protected:
	SharedPtr<Node> boulderNode;
	SharedPtr<Node> playerNode;
	float bob;
	float origin;
};

