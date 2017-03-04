#include "NPC.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <iostream>

#include "Actor.h"

using namespace Urho3D;

NPC::NPC(Context* context) : LogicComponent(context)
{
	
}

void NPC::RegisterObject(Context* context)
{
	context->RegisterFactory<NPC>();
}

void NPC::Start()
{
	modelNode = node_->GetChild("model");
	if (modelNode->GetPosition() != Vector3::ZERO)
	{
		std::cout << "NPC" << node_->GetID() << "'S MODEL GOT SCREWED UP SOMEWHERE." << std::endl;
		modelNode->SetPosition(Vector3::ZERO);
	}

	if (!node_->HasComponent<Actor>())
		actor = node_->CreateComponent<Actor>();
	else
		actor = node_->GetComponent<Actor>();

	if (!modelNode->HasComponent<AnimationController>())
		animController = modelNode->CreateComponent<AnimationController>();
	else
		animController = modelNode->GetComponent<AnimationController>();

	modelIndex = node_->GetVar("MODEL").GetInt();
	resourcePath = "Npcs/model"; resourcePath += modelIndex;
	actor->maxspeed = 5.0f;
	animatedModel = modelNode->GetComponent<AnimatedModel>();
	animController->Play(resourcePath + "/npc_stand.ani", 0, true, 0.5f);
	
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(NPC, OnCollision));
}

void NPC::FixedUpdate(float timeStep)
{
	actor->Move(false, false, false, false, false, timeStep);
}

void NPC::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
}

NPC::~NPC()
{
}
