#include "WeakChild.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <iostream>

WeakChild::WeakChild(Context* context) : LogicComponent(context), 
	matchPosition(false), matchRotation(false)
{
}

void WeakChild::RegisterObject(Context* context)
{
	context->RegisterFactory<WeakChild>();
}

void WeakChild::MakeWeakChild(Node* child, Node* parent, bool autoPos, bool autoRot)
{
	WeakChild* wk = child->CreateComponent<WeakChild>();
	wk->parent = parent;
	wk->matchPosition = autoPos;
	wk->matchRotation = autoRot;
}

void WeakChild::FixedUpdate(float timeStep)
{
	if (parent.Null() || parent.Get() == nullptr)
	{
		node_->Remove();
	}
	else
	{
		if (matchPosition) node_->SetWorldPosition(parent->GetWorldPosition());
		if (matchRotation) node_->SetWorldRotation(parent->GetWorldRotation());
		if (parent->GetParent() == nullptr) //This means that it's no longer in the scene
		{
			node_->Remove();
		}
	}
}

WeakChild::~WeakChild()
{
}
