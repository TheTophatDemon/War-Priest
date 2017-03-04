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
	
}

void NPC::FixedUpdate(float timeStep)
{
	
}

NPC::~NPC()
{
}
