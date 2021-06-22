#include "TempShield.h"

#include <Urho3D/Core/Context.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Audio/SoundSource3D.h>

#include <Urho3D/Resource/XMLFile.h>

TempShield::TempShield(Context* context) : LogicComponent(context),
	maxSize(18.0f),
	formed(false)
{
}

void TempShield::Start()
{
	ResourceCache* cache = GetSubsystem<ResourceCache>();

	//Spin those shields
	SharedPtr<ValueAnimation> spinAnim(new ValueAnimation(context_));
	spinAnim->SetKeyFrame(0.0f, Quaternion::IDENTITY);
	spinAnim->SetKeyFrame(0.5f, Quaternion(90.0f, Vector3::UP));
	spinAnim->SetKeyFrame(1.0f, Quaternion(180.0f, Vector3::UP));
	spinAnim->SetKeyFrame(1.5f, Quaternion(270.0f, Vector3::UP));
	spinAnim->SetKeyFrame(2.0f, Quaternion::IDENTITY);
	node_->SetAttributeAnimation("Rotation", spinAnim, WM_LOOP, 1.0f);
	node_->GetChild("subshield")->SetAttributeAnimation("Rotation", spinAnim, WM_LOOP, 2.0f);

	SharedPtr<ValueAnimation> fadeIn(new ValueAnimation(context_));
	fadeIn->SetKeyFrame(0.0f, 0.0f);
	fadeIn->SetKeyFrame(2.0f, 1.0f);

	SoundSource3D* windSound = node_->CreateComponent<SoundSource3D>();
	windSound->SetSoundType("GAMEPLAY");
	windSound->Play(cache->GetResource<Sound>("Sounds/enm_wind.wav"));
	windSound->SetDistanceAttenuation(0.0f, 60.0f, 2.0f);
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
	if (other != owner.Get() && !other->HasTag("handles_forcefields_manually") && otherBody->GetMass() > 0)
	{
		//All rigidbodies that are unlikely to have code to handle this interaction are pushed away
		const Vector3 diff = other->GetWorldPosition() - node_->GetWorldPosition();
		float push = 0.0f;
		if (diff != Vector3::ZERO)
		{
			push = 1.0f + (8.0f / diff.LengthSquared());
		}
		otherBody->ApplyImpulse(diff.Normalized() * push * otherBody->GetMass());
	}
}

Node* TempShield::MakeTempShield(Scene* scene, const Vector3 position, Node* owner)
{
	Node* n = scene->InstantiateXML(scene->GetSubsystem<ResourceCache>()->GetResource<XMLFile>("Objects/tempshield.xml")->GetRoot(), position, Quaternion::IDENTITY, LOCAL);
	TempShield* sh = new TempShield(scene->GetContext());
	sh->owner = owner;
	n->AddComponent(sh, 123, LOCAL);
	return n;
}

void TempShield::RegisterObject(Context* context)
{
	context->RegisterFactory<TempShield>();
}

TempShield::~TempShield()
{
}
