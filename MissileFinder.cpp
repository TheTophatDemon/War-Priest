#include "MissileFinder.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/UI/UIElement.h>
#include "Gameplay.h"
#include "Settings.h"
#include <iostream>

using namespace Urho3D;

int Indicator::leakCounter = 0;

MissileFinder::MissileFinder(Context* context) : LogicComponent(context),
	indicators(Vector<SharedPtr<Indicator>>())
{
}

void MissileFinder::RegisterObject(Context* context)
{
	context->RegisterFactory<MissileFinder>();
}

void MissileFinder::Start() 
{
	scene = GetScene();
	game = scene->GetComponent<Gameplay>();
	cache = GetSubsystem<ResourceCache>();
	baseElement = game->ourUI->CreateChild<UIElement>();
}

void MissileFinder::FixedUpdate(float timeStep)
{
	PODVector<Node*> missiles = scene->GetChildrenWithTag("missile", false);
	//Remove indicators for dead missiles
	for (Vector<SharedPtr<Indicator>>::Iterator i = indicators.Begin(); i != indicators.End(); i++)
	{		
		if ((*i)->missile.Get() && (*i)->sprite.Get())
		{
			if ((*i)->missile->HasComponent<StaticModel>()) continue; //The missile is invisible for a short period after exploding; don't keep track of it.
		}
		indicators.Remove(*i);
		i--; //Removing from the list causes indicators.End() to move back. Don't miss it!
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
			if (sm) 
			{
				if (!game->camera->GetFrustum().IsInsideFast(sm->GetWorldBoundingBox())) //We can't see it if it's outside the camera frustum
				{
					const float halfResX = Settings::GetResolutionX() / 2.0f;
					const float halfResY = Settings::GetResolutionY() / 2.0f;
					Vector3 screenCoords = game->camera->GetView() * missile->GetWorldPosition();
					if (!indicator)
					{
						Sprite* spr = baseElement->CreateChild<Sprite>();
						spr->SetFullImageRect();
						spr->SetSize(64, 64);
						spr->SetHotSpot(32, 32);
						spr->SetTexture(cache->GetResource<Texture2D>("UI/missile_indicator.png"));
						spr->SetPosition(halfResX, halfResY);
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
				}
				else if (indicator)
				{
					indicator->sprite->Remove();
				}
			}
		}
		else if (indicator)
		{
			indicator->sprite->Remove(); //This will cause the indicator to be removed next frame
		}
	}
}

void MissileFinder::Stop() 
{
	std::cout << "LEAK COUNTER FOR INDICATORS: " << Indicator::leakCounter << std::endl;
	baseElement->RemoveAllChildren();
	game->ourUI->RemoveChild(baseElement);
}

MissileFinder::~MissileFinder()
{
}
