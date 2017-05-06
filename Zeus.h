#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D\Graphics/Camera.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D\Graphics/DebugRenderer.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/Audio/Audio.h>

using namespace Urho3D;

class Zeus //Zeus is a dummy class that handles the creation of things such as explosions or projectiles.
		   //This serves mainly to provide global guidelines for creating effects that can be reused.
		   //Since the components may not have parameters in their constructors, this is neccesary.
{
public:
	Zeus();
	~Zeus();
};

