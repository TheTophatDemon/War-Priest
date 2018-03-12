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

class Zeus //He does all the special effects
{
public:
	Zeus();
	static Node* MakeLightBeam(Scene* scene, Vector3 position, const float height=64.0f);
	static Node* MakeShield(Scene* scene, Vector3 position, float radius);
	static Node* PuffOfSmoke(Scene* scene, Vector3 position, float length);
	static Node* MakeExplosion(Scene* scene, Vector3 position, const float life, const float size = 1.0f);
	static void ApplyRadialDamage(Scene* scene, Node* perpetrator, const float radius, const int damage, const int mask);
	~Zeus();
};

