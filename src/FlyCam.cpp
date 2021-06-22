/*
Copyright (C) 2021 Alexander Lunsford

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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

FlyCam::FlyCam(Context* context) : LogicComponent(context)
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
