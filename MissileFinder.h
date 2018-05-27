#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D\Scene\Scene.h>
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

using namespace Urho3D;

class Indicator : public RefCounted
{
public:
	static int leakCounter;
	Indicator() : sprite(nullptr), missile(nullptr) { leakCounter++; }
	Indicator(Sprite* spr, Node* miss) : sprite(spr), missile(miss) { leakCounter++; }
	~Indicator() 
	{
		if (sprite.Get()) sprite->Remove();
		leakCounter--;
	}
	WeakPtr<Sprite> sprite;
	WeakPtr<Node> missile;
};

class Gameplay;
class MissileFinder : public LogicComponent
{
	URHO3D_OBJECT(MissileFinder, LogicComponent);
public:
	MissileFinder(Context* context);
	static void RegisterObject(Context* context);
	virtual void Start();
	virtual void FixedUpdate(float timeStep);
	virtual void Stop();
	~MissileFinder();
protected:
	SharedPtr<Scene> scene;
	SharedPtr<ResourceCache> cache;
	SharedPtr<UIElement> baseElement;
	WeakPtr<Gameplay> game;
	Vector<SharedPtr<Indicator>> indicators;
};

