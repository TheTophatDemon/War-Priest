#include "RobeLocksBomb.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Audio/Sound.h>

#include "Settings.h"
#include "Zeus.h"

RobeLocksBomb::RobeLocksBomb(Context* context) : LogicComponent(context),
	lifeTimer(0.0f),
	tickTimer(1.0f),
	tickSpeed(1.0f)
{}

RobeLocksBomb::~RobeLocksBomb() {}

void RobeLocksBomb::RegisterObject(Context* context)
{
	context->RegisterFactory<RobeLocksBomb>();
}

void RobeLocksBomb::Start()
{
	SetUpdateEventMask(USE_FIXEDUPDATE);

	cache = GetSubsystem<ResourceCache>();

	tickSource = node_->GetOrCreateComponent<SoundSource3D>();
	tickSource->SetSoundType("GAMEPLAY");
}

void RobeLocksBomb::FixedUpdate(float timeStep)
{
	tickTimer += timeStep;
	if (tickTimer > tickSpeed)
	{
		tickSpeed *= 0.75f;
		tickTimer = 0.0f;
		tickSource->Play(cache->GetResource<Sound>("Sounds/enm_bomb.wav"), 44100.0f + Random(-500.0f, 500.0f), 0.75f);
	}

	lifeTimer += timeStep;
	if (lifeTimer > 4.0f)
	{
		Node* explosion = Zeus::MakeExplosion(GetScene(), node_->GetWorldPosition(), 3.0f, 5.5f);
		Zeus::ApplyRadialDamage(GetScene(), node_, 7.5f, Settings::ScaleWithDifficulty(7.0f, 10.0f, 13.0f), 132); //128 + 4

		SoundSource3D* s = explosion->CreateComponent<SoundSource3D>();
		s->SetSoundType("GAMEPLAY");
		s->Play(cache->GetResource<Sound>("Sounds/env_explode.wav"));

		node_->Remove();
	}
}

Node* RobeLocksBomb::MakeRobeLocksBomb(Scene* scene, const Vector3 position)
{
	Node* n = scene->InstantiateXML(scene->GetSubsystem<ResourceCache>()->GetResource<XMLFile>("Objects/robelocks_bomb.xml")->GetRoot(), 
		position, Quaternion::IDENTITY, LOCAL);
	n->CreateComponent<RobeLocksBomb>();
	return n;
}
