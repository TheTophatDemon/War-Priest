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
