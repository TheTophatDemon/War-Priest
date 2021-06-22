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

#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/RenderSurface.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Scene/ValueAnimation.h>

using namespace Urho3D;

class Indicator : public RefCounted
{
public:
	static int leakCounter;
	Indicator() : sprite(nullptr), projectile(nullptr), deleteMe(false) { leakCounter++; }
	Indicator(Sprite* spr, Node* miss) : sprite(spr), projectile(miss), deleteMe(false) { leakCounter++; }
	~Indicator() 
	{
		if (sprite.Get()) sprite->Remove();
		leakCounter--;
	}
	WeakPtr<Sprite> sprite;
	WeakPtr<Node> projectile;
	bool deleteMe;
};

class Gameplay;
class ProjectileWarner : public LogicComponent
{
	URHO3D_OBJECT(ProjectileWarner, LogicComponent);
public:
	ProjectileWarner(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	virtual void Stop();
	~ProjectileWarner();
protected:
	SharedPtr<Scene> scene;
	SharedPtr<ResourceCache> cache;
	SharedPtr<UIElement> baseElement;
	SharedPtr<ValueAnimation> fadeInAnimation;
	WeakPtr<Gameplay> game;
	Vector<SharedPtr<Indicator>> indicators;
};

