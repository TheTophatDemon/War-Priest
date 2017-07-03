#include "Launchpad.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <iostream>

#include "Actor.h"

Launchpad::Launchpad(Context* context) : LogicComponent(context)
{
	elapsedTime = 0.0f;
	launchForce = 42.0f;
	turnSpeedModifier = 0.0f;
}

void Launchpad::RegisterObject(Context* context)
{
	context->RegisterFactory<Launchpad>();
}

void Launchpad::Start()
{
	scene = GetScene();
	physworld = scene->GetComponent<PhysicsWorld>();
	cache = GetSubsystem<ResourceCache>();

	if (!node_->HasComponent<SoundSource3D>())
	{
		soundSource = node_->CreateComponent<SoundSource3D>();
	}
	else
	{
		soundSource = node_->GetComponent<SoundSource3D>();
	}
	soundSource->SetSoundType("GAMEPLAY");

	topNode = node_->GetChild("top");
	midNode = node_->GetChild("mid");
	baseNode = node_->GetChild("base");

	valAnim = new ValueAnimation(context_);
	valAnim->SetKeyFrame(0.0f, Color::BLACK);
	valAnim->SetKeyFrame(0.5f, Color::WHITE);
	valAnim->SetKeyFrame(1.5f, Color::BLACK);

	material = GetSubsystem<ResourceCache>()->GetResource<Material>("Materials/skins/launchpad_skin.xml");

	SubscribeToEvent(GetNode(), E_NODECOLLISIONSTART, URHO3D_HANDLER(Launchpad, OnCollision));
}

void Launchpad::FixedUpdate(float timeStep)
{
	elapsedTime += timeStep;
	baseNode->Rotate(Quaternion(timeStep * (32.0f + turnSpeedModifier), Vector3::UP), TS_LOCAL);
	midNode->Rotate(Quaternion(timeStep * (64.0f + turnSpeedModifier), Vector3::UP), TS_LOCAL);
	topNode->Rotate(Quaternion(timeStep * (128.0f + turnSpeedModifier), Vector3::UP), TS_LOCAL);
	
	if (turnSpeedModifier != 0.0f)
	{
		turnSpeedModifier *= 0.9f;
		if (fabs(turnSpeedModifier) < 0.1f)
			turnSpeedModifier = 0.0f;
	}

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
			turnSpeedModifier = 512.0f;
			soundSource->Play(cache->GetResource<Sound>("Sounds/itm_launchpad.wav"));
		}
	}
}

Launchpad::~Launchpad()
{
}
