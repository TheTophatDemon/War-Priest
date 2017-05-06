#include "Zeus.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Math/Ray.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D\Physics\PhysicsUtils.h>
#include <Urho3D\Resource\ResourceCache.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/View.h>
#include <Urho3D/Graphics/RenderPath.h>
#include <Urho3D/Graphics/ParticleEmitter.h>
#include <Urho3D/Graphics/ParticleEffect.h>
#include <Urho3D/Graphics/StaticModelGroup.h>
#include <Urho3D/Audio/SoundSource3D.h>
#include <Urho3D/Audio/SoundListener.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Scene/ObjectAnimation.h>
#include "TempEffect.h"

Zeus::Zeus()
{
}

Node* Zeus::MakeLightBeam(Scene* scene, Vector3 position)
{
	Vector3 beamSize = Vector3(3.5f, 64.0f, 3.5f);
	Vector3 closed = Vector3(0.0f, beamSize.y_, 0.0f);

	Node* lightColumn = scene->CreateChild();
	lightColumn->SetPosition(position);

	TempEffect* t = lightColumn->CreateComponent<TempEffect>();
	t->life = 0.75f;

	Node* smoker = scene->CreateChild();
	smoker->SetPosition(position);
	TempEffect* t2 = smoker->CreateComponent<TempEffect>();
	t2->life = 1.5f;

	ResourceCache* cache = scene->GetSubsystem<ResourceCache>();

	ParticleEmitter* emitter = smoker->CreateComponent<ParticleEmitter>();
	emitter->SetEffect(cache->GetResource<ParticleEffect>("Particles/smoke.xml"));
	emitter->SetScaled(false);

	SharedPtr<ValueAnimation> emitAnim(new ValueAnimation(scene->GetContext()));
	emitAnim->SetKeyFrame(0.0f, true);
	emitAnim->SetKeyFrame(0.1f, false);
	emitter->SetAttributeAnimation("Is Emitting", emitAnim, WM_CLAMP, 1.0f);

	StaticModel* model = lightColumn->CreateComponent<StaticModel>();
	model->SetModel(cache->GetResource<Model>("Models/Cylinder.mdl"));
	model->SetMaterial(cache->GetResource<Material>("Materials/lightbeam.xml"));

	SharedPtr<ObjectAnimation> columnAnimation(new ObjectAnimation(scene->GetContext()));

	SharedPtr<ValueAnimation> diffAnim(new ValueAnimation(scene->GetContext()));
	diffAnim->SetKeyFrame(0.0f, Color::WHITE);
	diffAnim->SetKeyFrame(0.25f, Color::WHITE);
	diffAnim->SetKeyFrame(0.75f, Color(0.0f, 0.0f, 0.0f, 0.0f));
	model->GetMaterial()->SetShaderParameterAnimation("MatDiffColor", diffAnim, WM_CLAMP, 1.0f);

	SharedPtr<ValueAnimation> scaleAnim(new ValueAnimation(scene->GetContext()));
	scaleAnim->SetKeyFrame(0.0f, closed);
	scaleAnim->SetKeyFrame(0.25f, beamSize);
	scaleAnim->SetKeyFrame(0.75f, closed);
	columnAnimation->AddAttributeAnimation("Scale", scaleAnim, WM_CLAMP, 1.0f);

	lightColumn->SetObjectAnimation(columnAnimation);
	return lightColumn;
}

Zeus::~Zeus()
{
}
