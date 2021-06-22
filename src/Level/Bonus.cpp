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

#include "Bonus.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <iostream>

using namespace Urho3D;

Bonus::Bonus(Context* context) : LogicComponent(context)
{
}

void Bonus::RegisterObject(Context* context)
{
	context->RegisterFactory<Bonus>();
}

void Bonus::Start()
{
	//Spinning
	SharedPtr<ValueAnimation> spinAnim = SharedPtr<ValueAnimation>(new ValueAnimation(context_));
	spinAnim->SetKeyFrame(0.0f, Quaternion::IDENTITY);
	spinAnim->SetKeyFrame(0.5f, Quaternion(90.0f, Vector3::UP));
	spinAnim->SetKeyFrame(1.0f, Quaternion(180.0f, Vector3::UP));
	spinAnim->SetKeyFrame(1.5f, Quaternion(270.0f, Vector3::UP));
	spinAnim->SetKeyFrame(2.0f, Quaternion(360.0f, Vector3::UP));
	node_->SetAttributeAnimation("Rotation", spinAnim, WM_LOOP, 1.0f);
	//Bobbing
	SharedPtr<ValueAnimation> bobAnim(new ValueAnimation(context_));
	bobAnim->SetInterpolationMethod(IM_SPLINE);
	bobAnim->SetSplineTension(0.1f);
	bobAnim->SetKeyFrame(0.0f, node_->GetWorldPosition());
	bobAnim->SetKeyFrame(1.0f, node_->GetWorldPosition() + Vector3::UP);
	bobAnim->SetKeyFrame(2.0f, node_->GetWorldPosition());
	node_->SetAttributeAnimation("Position", bobAnim, WM_LOOP, 1.0f);
	//Glow...uh...scaling
	SharedPtr<ValueAnimation> glowAnim(new ValueAnimation(context_));
	glowAnim->SetInterpolationMethod(IM_SPLINE);
	glowAnim->SetSplineTension(0.1f);
	glowAnim->SetKeyFrame(0.0f, Vector3(3.5f, 5.0f, 3.0f));
	glowAnim->SetKeyFrame(1.0f, Vector3(4.0f, 5.5f, 3.5f));
	glowAnim->SetKeyFrame(2.0f, Vector3(3.5f, 5.0f, 3.0f));
	Node* const glow = node_->GetChild("glow");
	glow->SetAttributeAnimation("Scale", glowAnim, WM_LOOP, 2.0f);
}

Bonus::~Bonus()
{
}
