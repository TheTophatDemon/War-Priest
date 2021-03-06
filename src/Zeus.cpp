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
#include <Urho3D/Physics/PhysicsUtils.h>
#include <Urho3D/Resource/ResourceCache.h>
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
#include <Urho3D/AngelScript/APITemplates.h>
#include <Urho3D/AngelScript/Script.h>
#include <Urho3D/AngelScript/ScriptAPI.h>
#include "TempEffect.h"
#include "WeakChild.h"
#include "Projectiles/Projectile.h"

#include <iostream>

Zeus::Zeus()
{
}

void Zeus::RegisterZeus(Context* context)
{
	asIScriptEngine* scrEngine = context->GetSubsystem<Script>()->GetScriptEngine();
	scrEngine->SetDefaultNamespace("Zeus");
	scrEngine->RegisterGlobalFunction("Node@+ MakeLightBeam(Scene@ scene, Vector3 position, const float height, const int sound=1)", asFUNCTION(Zeus::MakeLightBeam), asCALL_CDECL);
	scrEngine->SetDefaultNamespace("");
}

Node* Zeus::MakeLightBeam(Scene* scene, Vector3 position, const int sound)
{
	Node* lightColumn = scene->CreateChild();
	lightColumn->SetPosition(position);

	TempEffect* t = lightColumn->CreateComponent<TempEffect>();
	t->life = 1.5f;
	
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

	Vector3 beamSize = Vector3(3.5f, 1024.0f, 3.5f);
	Vector3 closed = Vector3(0.0f, beamSize.y_, 0.0f);

	SharedPtr<ValueAnimation> scaleAnim(new ValueAnimation(scene->GetContext()));
	scaleAnim->SetKeyFrame(0.0f, closed);
	scaleAnim->SetKeyFrame(0.25f, beamSize);
	scaleAnim->SetKeyFrame(0.75f, closed);
	columnAnimation->AddAttributeAnimation("Scale", scaleAnim, WM_CLAMP, 1.0f);

	if (sound > 0)
	{
		SoundSource3D* soundSource = lightColumn->CreateComponent<SoundSource3D>();
		soundSource->SetSoundType("GAMEPLAY");
		if (sound == 2)
		{
			soundSource->Play(cache->GetResource<Sound>("Sounds/env_beam2.wav"));
		}
		else
		{
			soundSource->Play(cache->GetResource<Sound>("Sounds/env_beam.wav"));
		}
	}

	lightColumn->SetObjectAnimation(columnAnimation);
	return lightColumn;
}

Node* Zeus::PuffOfSmoke(Scene* scene, Vector3 position, float length)
{
	ResourceCache* cache = scene->GetSubsystem<ResourceCache>();
	Node* p = scene->CreateChild();
	p->SetWorldPosition(position);
	ParticleEmitter* em = p->CreateComponent<ParticleEmitter>();
	em->SetEffect(cache->GetResource<ParticleEffect>("Particles/smoke.xml"));
	em->SetEmitting(true);
	TempEffect* te = new TempEffect(scene->GetContext());
	te->life = length;
	p->AddComponent(te, 666, LOCAL);
	return p;
}

Node* Zeus::MakeShield(Scene* scene, Vector3 position, float radius)
{
	ResourceCache* cache = scene->GetSubsystem<ResourceCache>();

	Node* shield = scene->CreateChild();
	shield->SetWorldPosition(position);
	shield->AddTag("shield");

	CollisionShape* cs = shield->CreateComponent<CollisionShape>();
	cs->SetSphere(1.0f, Vector3::ZERO, Quaternion::IDENTITY);

	RigidBody* rb = shield->CreateComponent<RigidBody>();
	rb->SetMass(0.0f);
	rb->SetCollisionLayer(16);
	rb->SetUseGravity(false);

	StaticModel* sm = shield->CreateComponent<StaticModel>();
	sm->SetModel(cache->GetResource<Model>("Models/Sphere.mdl"));
	sm->SetMaterial(cache->GetResource<Material>("Materials/shield.xml"));

	SharedPtr<ValueAnimation> sizeAnim(new ValueAnimation(scene->GetContext()));
	sizeAnim->SetKeyFrame(0.0f, Vector3(radius, radius, radius));
	sizeAnim->SetKeyFrame(0.2f, Vector3::ZERO);
	shield->SetAttributeAnimation("Scale", sizeAnim, WM_CLAMP, 1.0f);

	TempEffect* te = shield->CreateComponent<TempEffect>();
	te->life = 0.2f;

	return shield;
}

Node* Zeus::MakeExplosion(Scene* scene, Vector3 position, const float life, const float size)
{
	ResourceCache* cache = scene->GetSubsystem<ResourceCache>();

	Node* explosion = scene->CreateChild();
	explosion->SetWorldPosition(position);
	explosion->SetScale(size);

	ParticleEmitter* emitter = explosion->CreateComponent<ParticleEmitter>();
	emitter->SetEffect(cache->GetResource<ParticleEffect>("Particles/explosion.xml"));
	ParticleEmitter* flash = explosion->CreateComponent<ParticleEmitter>();
	flash->SetEffect(cache->GetResource<ParticleEffect>("Particles/flash.xml"));

	TempEffect* te = explosion->CreateComponent<TempEffect>();
	te->life = life;

	return explosion;
}

void Zeus::ApplyRadialDamage(Scene* scene, Node* perpetrator, const float radius, const int damage, const int mask)
{
	PhysicsWorld* physworld = scene->GetComponent<PhysicsWorld>();
	PODVector<RigidBody*> result;
	physworld->GetRigidBodies(result, Sphere(perpetrator->GetWorldPosition(), radius), mask);
	for (PODVector<RigidBody*>::Iterator i = result.Begin(); i != result.End(); ++i)
	{
		RigidBody* otherBody = (RigidBody*)*i;
		if (otherBody)
		{
			VariantMap map = VariantMap();
			map.Insert(Pair<StringHash, Variant>(Projectile::P_PERPETRATOR, perpetrator));
			map.Insert(Pair<StringHash, Variant>(Projectile::P_VICTIM, Variant(otherBody->GetNode())));
			map.Insert(Pair<StringHash, Variant>(Projectile::P_DAMAGE, damage));
			perpetrator->SendEvent(Projectile::E_PROJECTILEHIT, map);
		}
	}
}

Zeus::~Zeus()
{
}
