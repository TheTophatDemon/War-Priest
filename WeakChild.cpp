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

void WeakChild::Start()
{
	SubscribeToEvent(E_NODEREMOVED, URHO3D_HANDLER(WeakChild, OnParentRemoved));
}

void WeakChild::OnParentRemoved(StringHash eventName, VariantMap& eventData)
{
	Node* node = dynamic_cast<Node*>(eventData["Node"].GetPtr());
	if (node == parent.Get())
	{
		node_->Remove();
	}
}

WeakChild::~WeakChild()
{
}
