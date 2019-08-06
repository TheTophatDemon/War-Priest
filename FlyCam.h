#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/AngelScript/APITemplates.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Graphics/Renderer.h>

using namespace Urho3D;

class FlyCam : public LogicComponent
{
	URHO3D_OBJECT(FlyCam, LogicComponent);
public:
	FlyCam(Context* context);
	~FlyCam();
	static void RegisterObject(Context* context);
	virtual void Start() override;
	virtual void FixedUpdate(float timeStep) override;
protected:
	void OnRemoved(StringHash eventType, VariantMap& eventData);

	SharedPtr<Input> input;
	SharedPtr<Renderer> renderer;
	SharedPtr<Camera> camera;
	SharedPtr<Camera> oldCamera;
	PODVector<Node*> waypoints;
	Vector3 velocity;
	Vector2 angularVelocity;
	WeakPtr<Node> nextWaypoint;
	int waypointIndex;
};

