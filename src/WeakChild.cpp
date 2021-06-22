/*
Copyright (C) 2021 Alexander Lunsford

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
