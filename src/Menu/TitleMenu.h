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

#include "Menu.h"

class TitleMenu : public GP::Menu
{
	URHO3D_OBJECT(TitleMenu, GP::Menu);
public:
	TitleMenu(Context* context, TitleScreen* ts, SharedPtr<Gameplay> gm);
	virtual void OnEnter() override;
	virtual void Update(float timeStep) override;
	~TitleMenu();
protected:
	void OnEvent(StringHash eventType, VariantMap& eventData);
	SharedPtr<Button> resumeButton;
};

