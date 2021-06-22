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

#include "GravityPlate.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/PhysicsEvents.h>

#include "../Gameplay.h"
#include "../GunPriest.h"

GravityPlate::GravityPlate(Context* context) : LogicComponent(context),
	rotationDirection(Quaternion::IDENTITY),
	rotationAxis(Vector3::ZERO),
	rotationForce(1.0f)
{
}

void GravityPlate::RegisterObject(Context* context)
{
	context->RegisterFactory<GravityPlate>();
}

void GravityPlate::Start()
{
	game = GetScene()->GetComponent<Gameplay>();
	body = node_->GetComponent<RigidBody>();
}

void GravityPlate::FixedUpdate(float timeStep)
{
	rotationDirection.FromAngleAxis(rotationForce * timeStep, rotationAxis);
	node_->Rotate(rotationDirection, TS_LOCAL);
}

GravityPlate::~GravityPlate()
{
}
