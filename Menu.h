#pragma once

#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D\Scene\Scene.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/Button.h>
#include "Gameplay.h"

class TitleScreen;
namespace GP 
{
	class Menu : public Object
	{
		URHO3D_OBJECT(Menu, Object);
	public:
		Menu(Context* context, TitleScreen* ts, SharedPtr<Gameplay> gm);
		virtual void OnEnter();
		virtual void OnLeave();
		virtual void Update(float timeStep);
		virtual ~Menu();
		String layoutPath;
	protected:
		SharedPtr<TitleScreen> titleScreen;
		SharedPtr<Gameplay> gameplay;
		SharedPtr<ResourceCache> cache;
		SharedPtr<UI> ui;
		void DisableTexts();
	};
}

