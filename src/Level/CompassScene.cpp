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

#include "CompassScene.h"

#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Resource/Image.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Graphics/Zone.h>
#include <iostream>

CompassScene::CompassScene(Context* context) : Object(context)
{
	compassRotation = 0.0f;
}

void CompassScene::Start()
{
	cache = GetSubsystem<ResourceCache>();
	scene = new Scene(context_);
	scene->CreateComponent<Octree>();

	//scene->LoadXML(cache->GetResource<XMLFile>("Scenes/crossroads.xml")->GetRoot());

	cameraNode = scene->CreateChild();
	cameraNode->SetWorldPosition(Vector3(3.0f, 3.25f, 0.0f));
	Quaternion q = Quaternion();
	q.FromEulerAngles(50.0f, -90.0f, 0.0f);
	cameraNode->SetWorldRotation(q);

	Camera* camera = cameraNode->CreateComponent<Camera>();
	camera->SetFov(45.0f);

	compass = scene->CreateChild();
	StaticModel* sm = compass->CreateComponent<StaticModel>();
	sm->SetModel(cache->GetResource<Model>("Models/compass.mdl"));
	sm->SetMaterial(cache->GetResource<Material>("Materials/skins/compass_skin.xml"));

	viewport = new Viewport(context_, scene, camera, 0);
	
	Zone* z = scene->CreateComponent<Zone>();
	z->SetFogColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
	z->SetAmbientColor(Color(0.0f, 0.0f, 0.0f, 0.0f));

	renderedTexture = new Texture2D(context_);
	renderedTexture->SetSize(128, 128, Graphics::GetRGBAFormat(), TEXTURE_RENDERTARGET);
	renderedTexture->SetFilterMode(TextureFilterMode::FILTER_BILINEAR);

	RenderSurface* rs = renderedTexture->GetRenderSurface();
	rs->SetUpdateMode(RenderSurfaceUpdateMode::SURFACE_UPDATEALWAYS);
	rs->SetViewport(0, viewport);

	//cache->GetResource<Material>("Materials/grass.xml")->SetTexture(TU_DIFFUSE, renderedTexture);

	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CompassScene, Update));
}

void CompassScene::RegisterObject(Context* context)
{
	context->RegisterFactory<CompassScene>();
}

void CompassScene::Update(StringHash eventType, VariantMap& eventData)
{
	compass->SetWorldRotation(compass->GetWorldRotation().Slerp(Quaternion(compassRotation + 180.0f, Vector3::UP), 0.5f));
}

CompassScene::~CompassScene()
{
}
