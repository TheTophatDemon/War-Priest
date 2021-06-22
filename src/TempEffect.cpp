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
	
}

void TempEffect::FixedUpdate(float timeStep)
{
	timer += timeStep;
	if (timer > life)
	{
		node_->Remove();
	}
}

TempEffect::~TempEffect()
{
}
