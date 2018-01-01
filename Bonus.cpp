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
	glowAnim->SetKeyFrame(0.0f, Vector3(1.05f, 1.05f, 1.05f));
	glowAnim->SetKeyFrame(1.0f, Vector3(1.25f, 1.25f, 1.25f));
	glowAnim->SetKeyFrame(2.0f, Vector3(1.05f, 1.05f, 1.05f));
	Node* const glow = node_->GetChild("glow");
	glow->SetAttributeAnimation("Scale", glowAnim, WM_LOOP, 1.0f);
}

Bonus::~Bonus()
{
}
