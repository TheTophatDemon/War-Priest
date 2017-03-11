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
	boulderNode = GetScene()->GetComponent<Gameplay>()->boulderNode;
	playerNode = GetScene()->GetComponent<Gameplay>()->playerNode;
}

void Cross::FixedUpdate(float timeStep)
{
	bob += timeStep;
	
	Vector3 difference = (boulderNode->GetWorldPosition() - node_->GetWorldPosition());
	float distanceFromBoulder = difference.Length();
	if (distanceFromBoulder < 3.0f)
	{
		node_->GetScene()->GetComponent<Gameplay>()->FlashScreen(Color(0.7f, 0.7f, 0.8f, 0.85f), 0.025f);
		playerNode->SetVar("Cross Count", playerNode->GetVar("Cross Count").GetInt() + 1);
		node_->Remove();
	}
	else if (distanceFromBoulder < 6.0f)
	{
		node_->Translate(difference.Normalized() * 0.5f, TS_WORLD);
	}
	else
	{
		node_->Rotate(Quaternion(timeStep * 75.0f, Vector3::UP));
		node_->SetPosition(Vector3(node_->GetPosition().x_, origin + (sinf(bob * 5.0f)) * 0.25f, node_->GetPosition().z_));
	}
}

Cross::~Cross()
{
}
