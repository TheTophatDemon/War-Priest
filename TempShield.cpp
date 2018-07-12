#include "TempShield.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Audio/SoundSource3D.h>

TempShield::TempShield(Context* context) : LogicComponent(context),
	maxSize(18.0f),
	formed(false)
{
}

void TempShield::Start()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();

	node_->AddTag("tempshield");
	CollisionShape* cs = node_->CreateComponent<CollisionShape>();
	cs->SetSphere(1.0f);
	RigidBody* rb = node_->CreateComponent<RigidBody>();
	rb->SetCollisionLayer(17);//1+16
	rb->SetTrigger(true);
	rb->SetLinearFactor(Vector3::ZERO);
	rb->SetAngularFactor(Vector3::ZERO);

	model = node_->CreateComponent<StaticModel>();
	model->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
	model->SetMaterial(cache->GetResource<Material>("Materials/shield.xml"));
	node_->SetScale(0.5f);
	subShield = node_->CreateChild("subshield");
	StaticModel* subModel = (StaticModel*)subShield->CloneComponent(model);
	subShield->SetScale(0.95f);

	//Spin those shields
	SharedPtr<ValueAnimation> spinAnim(new ValueAnimation(context_));
	spinAnim->SetKeyFrame(0.0f, Quaternion::IDENTITY);
	spinAnim->SetKeyFrame(0.5f, Quaternion(90.0f, Vector3::UP));
	spinAnim->SetKeyFrame(1.0f, Quaternion(180.0f, Vector3::UP));
	spinAnim->SetKeyFrame(1.5f, Quaternion(270.0f, Vector3::UP));
	spinAnim->SetKeyFrame(2.0f, Quaternion::IDENTITY);
	node_->SetAttributeAnimation("Rotation", spinAnim, WM_LOOP, 1.0f);
	subShield->SetAttributeAnimation("Rotation", spinAnim, WM_LOOP, 2.0f);

	SharedPtr<ValueAnimation> fadeIn(new ValueAnimation(context_));
	fadeIn->SetKeyFrame(0.0f, 0.0f);
	fadeIn->SetKeyFrame(2.0f, 1.0f);

	SoundSource3D* windSound = node_->CreateComponent<SoundSource3D>();
	windSound->SetSoundType("GAMEPLAY");
	windSound->Play(cache->GetResource<Sound>("Sounds/enm_wind.wav"));
	windSound->SetDistanceAttenuation(0.0f, 100.0f, 2.0f);
	windSound->SetAttributeAnimation("Gain", fadeIn, WM_CLAMP, 1.0f);

	SubscribeToEvent(node_, E_NODECOLLISION, URHO3D_HANDLER(TempShield, OnCollision));
}

void TempShield::FixedUpdate(float timeStep)
{
	if (node_->GetScale().x_ < maxSize)
	{
		node_->SetScale(node_->GetScale() + Vector3(timeStep * 15.0f, timeStep * 15.0f, timeStep * 15.0f));
	}
	else
	{
		node_->SetScale(maxSize);
		formed = true;
	}
}

void TempShield::OnCollision(StringHash eventType, VariantMap& eventData)
{
	Node* other = (Node*)eventData["OtherNode"].GetPtr();
	RigidBody* otherBody = (RigidBody*)eventData["OtherBody"].GetPtr();
	if (other != node_ && otherBody->GetCollisionLayer() <= 1 && otherBody->GetMass() > 0)
	{
		//All rigidbodies that are unlikely to have code to handle this interaction are pushed away
		const Vector3 diff = other->GetWorldPosition() - node_->GetWorldPosition();
		const float push = 3.0f + (12.0f / diff.LengthSquared());
		otherBody->ApplyImpulse(diff.Normalized() * Vector3(push, push, push) * otherBody->GetMass());
	}
}

void TempShield::RegisterObject(Context* context)
{
	context->RegisterFactory<TempShield>();
}

TempShield::~TempShield()
{
}
