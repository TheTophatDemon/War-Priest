#include "Boulder.h"


Boulder::Boulder(Context* context) : LogicComponent(context)
{

}

void Boulder::RegisterObject(Context* context)
{
	context->RegisterFactory<Boulder>();
}

void Boulder::Start()
{
	model = node_->GetComponent<StaticModel>();
	flashAnim = new ValueAnimation(context_);
	flashAnim->SetKeyFrame(0.0f, Color::WHITE);
	flashAnim->SetKeyFrame(0.5f, Color::BLACK);
}

void Boulder::Flash()
{
	model->GetMaterial(0U)->SetShaderParameterAnimation("MatEmissiveColor", flashAnim, WM_ONCE, 1.0f);
}

void Boulder::FixedUpdate(float timeStep)
{
	
}

Boulder::~Boulder()
{
}
