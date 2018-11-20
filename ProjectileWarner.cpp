#include "ProjectileWarner.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/Text.h>
#include "Gameplay.h"
#include "Settings.h"
#include <iostream>

using namespace Urho3D;

int Indicator::leakCounter = 0;

ProjectileWarner::ProjectileWarner(Context* context) : LogicComponent(context),
	indicators(Vector<SharedPtr<Indicator>>())
{
	fadeInAnimation = SharedPtr<ValueAnimation>(new ValueAnimation(context_));
	fadeInAnimation->SetKeyFrame(0.0f, Color(1.0f, 1.0f, 1.0f, 0.0f));
	fadeInAnimation->SetKeyFrame(0.25f, Color::WHITE);
}

void ProjectileWarner::RegisterObject(Context* context)
{
	context->RegisterFactory<ProjectileWarner>();
}

void ProjectileWarner::Start() 
{
	scene = GetScene();
	game = scene->GetComponent<Gameplay>();
	cache = GetSubsystem<ResourceCache>();
	baseElement = game->ourUI->CreateChild<UIElement>();
	baseElement->SetAlignment(HA_LEFT, VA_TOP);
	baseElement->SetPosition(0.0f, 0.0f);
}

void ProjectileWarner::FixedUpdate(float timeStep)
{
	PODVector<Node*> missiles = scene->GetChildrenWithTag("missile", true);
	//Update indicators
	for (Vector<SharedPtr<Indicator>>::Iterator i = indicators.Begin(); i != indicators.End(); i++)
	{	
		Indicator* ind = (Indicator*)*i;
		if (ind) 
		{
			if (!ind->sprite.Get())
			{
				ind->deleteMe = true;
			}
			if (ind->missile.Get())
			{
				//Projectiles remove their "projectile" tag once they've hit something and are no longer a threat.
				if (!ind->missile->HasTag("projectile")) ind->deleteMe = true;
			}
			else
			{
				ind->deleteMe = true;
			}
			if (ind->deleteMe == true)
			{
				if (!ind->sprite.Get()) goto remove;
				if (ind->sprite->GetAttributeAnimationTime("Color") > fadeInAnimation->GetEndTime())
				{
					ind->sprite->SetAttributeAnimationTime("Color", fadeInAnimation->GetEndTime());
				}
				ind->sprite->SetAttributeAnimationSpeed("Color", -1.0f); //Reverse the fade in animation
				if (ind->sprite->GetColor(Corner::C_TOPLEFT).a_ <= 0.0f)
				{
					goto remove;
				}
				continue;
			remove:
				indicators.Remove(*i);
				i--; //This prevents the iterator from skipping over the last item
			}
		}
	}
	for (Node* missile : missiles)
	{
		//Find indicator for current missile
		Indicator* indicator = nullptr;
		for (SharedPtr<Indicator>& ind : indicators)
		{
			if (ind->missile.Get() == missile)
			{
				indicator = ind.Get();
				break;
			}
		}
		const float distance = (missile->GetWorldPosition() - node_->GetWorldPosition()).Length();
		if (distance < 120.0f) 
		{
			StaticModel* sm = missile->GetComponent<StaticModel>();
			//Automatically generates a small bounding box for projectiles without models
			BoundingBox boundingBox = BoundingBox(missile->GetWorldPosition() - Vector3::ONE, missile->GetWorldPosition() + Vector3::ONE);
			if (sm) 
			{
				boundingBox = sm->GetWorldBoundingBox();
			}
			if (!game->camera->GetFrustum().IsInsideFast(boundingBox)) //We can't see it if it's outside the camera frustum
			{
				//UI is scaled from 1280x720 to fit the current resolution, but we must make corrections for different aspect ratios.
				const float halfResX = game->ourUI->GetWidth() / 2.0f;
				const float halfResY = game->ourUI->GetHeight() / 2.0f;
				Vector3 screenCoords = game->camera->GetView() * missile->GetWorldPosition();
				Vector3 screenDirection = game->camera->GetView().Rotation() * missile->GetWorldDirection();
				if (!indicator)
				{
					Sprite* spr = baseElement->CreateChild<Sprite>();
					spr->SetFullImageRect();
					spr->SetSize(64, 64);
					spr->SetHotSpot(32, 32);
					spr->SetTexture(cache->GetResource<Texture2D>("UI/missile_indicator.png"));
					spr->SetPosition(halfResX, halfResY);
					spr->SetAttributeAnimation("Color", fadeInAnimation, WM_CLAMP, 1.0f);
					indicator = new Indicator(spr, missile);
					indicators.Push(SharedPtr<Indicator>(indicator));
				}
				screenCoords.Normalize();
				screenCoords.y_ = -screenCoords.y_;
				if (fabs(screenCoords.x_) > fabs(screenCoords.y_))
				{
					screenCoords.x_ = Sign(screenCoords.x_);
					screenCoords.y_ = screenCoords.y_ / fabs(screenCoords.x_);
				}
				else
				{
					screenCoords.x_ = screenCoords.x_ / fabs(screenCoords.y_);
					screenCoords.y_ = Sign(screenCoords.y_);
				}
				indicator->sprite->SetPosition(screenCoords.x_ * (halfResX - indicator->sprite->GetHotSpot().x_) + halfResX,
					screenCoords.y_ * (halfResY - indicator->sprite->GetHotSpot().y_) + halfResY);
				indicator->sprite->SetRotation(Atan2(screenDirection.x_, screenDirection.y_));
			}
			else if (indicator)
			{
				indicator->deleteMe = true;
			}
		}
		else if (indicator)
		{
			indicator->deleteMe = true;
		}
	}
}

void ProjectileWarner::Stop() 
{
	//std::cout << "LEAK COUNTER FOR INDICATORS: " << Indicator::leakCounter << std::endl;
	baseElement->RemoveAllChildren();
	game->ourUI->RemoveChild(baseElement);
}

ProjectileWarner::~ProjectileWarner()
{
}
