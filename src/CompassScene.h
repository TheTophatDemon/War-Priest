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

using namespace Urho3D;

class CompassScene : public Object
{
	URHO3D_OBJECT(CompassScene, Object);
public:
	CompassScene(Context* context);
	static void RegisterObject(Context* context);
	void Start();
	void Update(StringHash eventType, VariantMap& eventData);
	~CompassScene();
	SharedPtr<Texture2D> renderedTexture;
	float compassRotation;
protected:
	SharedPtr<Node> compass;
	SharedPtr<Node> cameraNode;
	SharedPtr<Scene> scene;
	SharedPtr<ResourceCache> cache;
	SharedPtr<Viewport> viewport;
};

