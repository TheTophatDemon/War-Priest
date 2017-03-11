#include "TempEffect.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/IO/MemoryBuffer.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/SceneEvents.h>

using namespace Urho3D;

TempEffect::TempEffect(Context* context) : LogicComponent(context)
{

}

void TempEffect::RegisterObject(Context* context)
{
	context->RegisterFactory<TempEffect>();
}

void TempEffect::Start()
{
	emitter = node_->CreateComponent<ParticleEmitter>();
	emitter->SetEffect(GetSubsystem<ResourceCache>()->GetResource<ParticleEffect>(node_->GetVar("EFFECT NAME").GetString()));
	emitter->ResetEmissionTimer();
	emitter->SetEmitting(true);
}

void TempEffect::FixedUpdate(float timeStep)
{
	timer += timeStep;
	float life = node_->GetVar("TIME").GetFloat();
	if (timer > life)
	{
		emitter->SetEmitting(false);
		if (timer > life + 4.0f)
		{
			node_->Remove();
		}
	}
}

TempEffect::~TempEffect()
{
}
