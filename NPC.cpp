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
#include <Urho3D/Graphics/AnimationState.h>
#include <Urho3D/Graphics/Animation.h>
#include <iostream>

#include "Actor.h"
#include "Gameplay.h"

using namespace Urho3D;

#define STATE_IDLE 0
#define STATE_WALK 1

#define TURNAMOUNT 5.0f

NPC::NPC(Context* context) : LogicComponent(context)
{
	state = STATE_IDLE;
	stateTimer = 0;
	modelIndex = 0;
	turnTimer = Random(5, 250);
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

	game = GetScene()->GetComponent<Gameplay>();
	
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(NPC, OnCollision));
}

void NPC::FixedUpdate(float timeStep)
{
	float dist = (node_->GetWorldPosition() - game->playerNode->GetWorldPosition()).Length();
	if (dist < 250.0f) 
	{
		stateTimer -= 1;
		switch (state)
		{
		case STATE_WALK:
			actor->Move(true, false, false, false, false, timeStep);
			if (stateTimer < 0)
				ChangeState(STATE_IDLE, Random(50, 200));
			break;
		default:
			actor->Move(false, false, false, false, false, timeStep);
			//animController->Play(resourcePath + "/npc_stand.ani", 0, true, 0.5f);
			if (stateTimer < 0)
				ChangeState(STATE_WALK, Random(50, 200));
			break;
		}

		if (state == STATE_WALK || state == STATE_IDLE)
		{
			if (turn != 0.0f)
			{
				node_->Rotate(Quaternion(Sign(turn) * TURNAMOUNT, Vector3::UP));
				if (turn > 0.0f)
				{
					turn -= TURNAMOUNT;
				}
				else
				{
					turn += TURNAMOUNT;
				}
				if (fabs(turn) <= TURNAMOUNT)
				{
					turn = 0.0f;
				}
			}
			else
			{
				turnTimer -= 1;
				if (turnTimer < 0)
				{
					turnTimer = Random(5, 250);
					turn = Random(-135.0f, 135.0f);
				}
			}
		}
	}
}

void NPC::ChangeState(int newState, int timer)
{
	state = newState;
	stateTimer = timer;
	switch (newState)
	{
	case STATE_WALK:
		animController->PlayExclusive(resourcePath + "/npc_walk.ani", 0, true, 0.5f);
		break;
	default:
		animController->PlayExclusive(resourcePath + "/npc_stand.ani", 0, true, 0.5f);
		break;
	}
}

void NPC::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (state == STATE_WALK) 
	{
		VectorBuffer contacts = eventData["Contacts"].GetBuffer();
		while (!contacts.IsEof())
		{
			Vector3 position = contacts.ReadVector3();
			Vector3 normal = contacts.ReadVector3();
			float distance = contacts.ReadFloat();
			float impulse = contacts.ReadFloat();
			if (fabs(normal.x_) > 0.01f || fabs(normal.z_) > 0.01f)
			{
				turn = 90.0f;
			}
		}
	}
}

NPC::~NPC()
{
}
