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

