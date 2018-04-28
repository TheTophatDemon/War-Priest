#include "WeakChild.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <iostream>

WeakChild::WeakChild(Context* context) : LogicComponent(context)
{
}

void WeakChild::RegisterObject(Context* context)
{
	context->RegisterFactory<WeakChild>();
}

void WeakChild::MakeWeakChild(Node* child, Node* parent)
{
	WeakChild* wk = child->CreateComponent<WeakChild>();
	wk->parent = parent;
}

void WeakChild::FixedUpdate(float timeStep)
{
	if (parent.Null() || parent.Get() == nullptr)
	{
		node_->Remove();
	}
	else
	{
		if (parent->GetParent() == nullptr) //This means that it's no longer in the scene
		{
			node_->Remove();
		}
	}
}

WeakChild::~WeakChild()
{
}
