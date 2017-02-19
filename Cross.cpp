#include "Cross.h"
#include "Gameplay.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/StaticModel.h>

using namespace Urho3D;

Cross::Cross(Context* context) : LogicComponent(context)
{
	bob = 0.0f;
}

void Cross::RegisterObject(Context* context)
{
	context->RegisterFactory<Cross>();
}

void Cross::Start()
{
	origin = node_->GetPosition().y_;
	playerNode = node_->GetScene()->GetChild("player");
}

void Cross::FixedUpdate(float timeStep)
{
	bob += timeStep;
	node_->Rotate(Quaternion(timeStep * 75.0f, Vector3::UP));
	node_->SetPosition(Vector3(node_->GetPosition().x_, origin + (sinf(bob * 5.0f)) * 0.25f, node_->GetPosition().z_));
	if ((playerNode->GetWorldPosition() - node_->GetWorldPosition()).Length() < 1.0f)
	{
		playerNode->SetVar("Cross Count", playerNode->GetVar("Cross Count").GetInt() + 1);
		node_->Remove();
	}
}

Cross::~Cross()
{
}
