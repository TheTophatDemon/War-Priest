#include "Launchpad.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <iostream>

#include "Actor.h"

Launchpad::Launchpad(Context* context) : LogicComponent(context)
{
	elapsedTime = 0.0f;
	launchForce = 42.0f;
}

void Launchpad::RegisterObject(Context* context)
{
	context->RegisterFactory<Launchpad>();
}

void Launchpad::Start()
{
	scene = GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	if (!node_->HasComponent<SoundSource3D>())
	{
		soundSource = node_->CreateComponent<SoundSource3D>();
	}
	else
	{
		soundSource = node_->GetComponent<SoundSource3D>();
	}

	topNode = node_->GetChild("top");
	midNode = node_->GetChild("mid");
	baseNode = node_->GetChild("base");

	SubscribeToEvent(GetNode(), E_NODECOLLISIONSTART, URHO3D_HANDLER(Launchpad, OnCollision));
}

void Launchpad::FixedUpdate(float timeStep)
{
	elapsedTime += timeStep;
	baseNode->Rotate(Quaternion(timeStep * 32.0f, Vector3::UP), TS_LOCAL);
	midNode->Rotate(Quaternion(timeStep * 64.0f, Vector3::UP), TS_LOCAL);
	topNode->Rotate(Quaternion(timeStep * 128.0f, Vector3::UP), TS_LOCAL);
	
	topNode->Translate(Vector3(0.0f, -sinf(elapsedTime * 4.0f) * 0.01f, 0.0f));
}

void Launchpad::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* n = dynamic_cast<Node*>(eventData["OtherNode"].GetPtr());
	if (n)
	{
		if (n->HasComponent<Actor>()) 
		{
			Actor* act = n->GetComponent<Actor>();
			act->fall = launchForce - (Min(act->fall, 0.0f) * 0.2f);
		}
	}
}

Launchpad::~Launchpad()
{
}
