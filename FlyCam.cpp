#include "FlyCam.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/Viewport.h>
#include <iostream>

#include "Settings.h"

using namespace Urho3D;

FlyCam::FlyCam(Context* context) : LogicComponent(context),
	waypointIndex(0)
{
}

void FlyCam::RegisterObject(Context* context)
{
	context->RegisterFactory<FlyCam>();
}

void FlyCam::Start()
{
	input = GetSubsystem<Input>();
	renderer = GetSubsystem<Renderer>();
	
	camera = node_->CreateComponent<Camera>();
	camera->SetFov(70.0f);
	oldCamera = renderer->GetViewport(0)->GetCamera();
	renderer->GetViewport(0)->SetCamera(camera);

	waypoints = GetScene()->GetChildrenWithTag("camera_waypoint", true);

	SubscribeToEvent(node_, E_NODEREMOVED, URHO3D_HANDLER(FlyCam, OnRemoved));
}

void FlyCam::FixedUpdate(float timeStep)
{
	bool shift = input->GetKeyDown(KEY_RSHIFT);

	//Controls
	Vector3 movement = Vector3(
		(input->GetKeyDown(KEY_L) ? 1 : 0) - (input->GetKeyDown(KEY_J) ? 1 : 0),
		(input->GetKeyDown(KEY_U) ? 1 : 0) - (input->GetKeyDown(KEY_O) ? 1 : 0),
		(input->GetKeyDown(KEY_I) ? 1 : 0) - (input->GetKeyDown(KEY_K) ? 1 : 0)
	);
	movement.Normalize();

	velocity += (node_->GetWorldRotation() * movement) * 2.5f * timeStep;

	//Return to player view
	if (input->GetKeyPress(KEY_M))
	{
		renderer->GetViewport(0)->SetCamera(oldCamera);
		node_->Remove();
		return;
	}

	//Go to next waypoint
	if (input->GetKeyDown(KEY_N))
	{
		for (int i = 0; i < waypoints.Size(); ++i)
		{
			if (waypoints[i]->GetName().Contains(String(waypointIndex)))
			{
				nextWaypoint = waypoints[i];
				break;
			}
		}
		const Vector3 diff = (nextWaypoint->GetWorldPosition() - node_->GetWorldPosition());
		node_->Translate(diff * 0.5f * timeStep, TS_WORLD);
		Quaternion lookAt;
		lookAt.FromLookRotation(diff, Vector3::UP);
		node_->SetWorldRotation(node_->GetWorldRotation().Slerp(nextWaypoint->GetWorldRotation(), 0.2f));
		if (diff.Length() < 0.1f)
		{
			node_->SetWorldPosition(nextWaypoint->GetWorldPosition());
			node_->SetWorldRotation(nextWaypoint->GetWorldRotation());
			++waypointIndex;
			if (waypointIndex >= waypoints.Size()) waypointIndex = 0;
		}
	}

	float oldLen = velocity.Length();
	float newLen = Clamp(oldLen - 1.0f * timeStep, 0.0f, shift ? 0.5f : 1.0f);
	if (oldLen != 0.0f) velocity = newLen * velocity / oldLen;
	node_->Translate(velocity, TS_WORLD);

	angularVelocity += Vector2(input->GetMouseMoveX() * 0.3f, input->GetMouseMoveY() * 0.3f) * 5.0f * timeStep;
	oldLen = angularVelocity.Length();
	newLen = Clamp(oldLen - 2.5f * timeStep, 0.0f, 2.0f);
	if (oldLen != 0.0f) angularVelocity = newLen * angularVelocity / oldLen;
	node_->Rotate(Quaternion(angularVelocity.y_, Vector3::RIGHT), TS_LOCAL);
	node_->Rotate(Quaternion(angularVelocity.x_, Vector3::UP), TS_WORLD);
}

void FlyCam::OnRemoved(StringHash eventType, VariantMap& eventData)
{
	
}

FlyCam::~FlyCam()
{
}
