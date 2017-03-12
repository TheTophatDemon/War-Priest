#include "Boulder.h"
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include "Player.h"

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
	body = node_->GetComponent<RigidBody>();
	flashAnim = new ValueAnimation(context_);
	flashAnim->SetKeyFrame(0.0f, Color::BLUE);
	flashAnim->SetKeyFrame(0.5f, Color(Color::WHITE, 0.85f));
	SubscribeToEvent(GetNode(), E_NODECOLLISION, URHO3D_HANDLER(Boulder, OnCollision));
}

void Boulder::Flash()
{
	model->GetMaterial(0U)->SetShaderParameterAnimation("MatDiffColor", flashAnim, WM_ONCE, 1.0f);
}

void Boulder::FixedUpdate(float timeStep)
{
	
}

void Boulder::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	if (other->HasComponent<Player>())
	{
		body->SetLinearVelocity(Vector3(0.0f, body->GetLinearVelocity().y_, 0.0f));
	}
}

Boulder::~Boulder()
{
}
