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

#include "Sign.h"

#include <Urho3D/Core/Context.h>

#include "../Gameplay.h"
#include "../Settings.h"

#define READ_DISTANCE 8 * 8

using namespace GP;

Sign::Sign(Context* context) : LogicComponent(context){}

void Sign::RegisterObject(Context* context)
{
	context->RegisterFactory<Sign>();
}

void Sign::Start()
{
	game = GetScene()->GetComponent<Gameplay>();
	input = GetSubsystem<Input>();
}

void Sign::FixedUpdate(float timeStep)
{
	const float distSquared = (game->playerNode->GetWorldPosition() - node_->GetWorldPosition()).LengthSquared();
	if (distSquared < READ_DISTANCE)
	{
		game->DisplayMessage(ParseMessage(node_->GetVar("message").GetString()), Color::WHITE, 0.5f, 5);
	}
}

String Sign::ParseMessage(const String& message)
{
	String newMessage = "";
	StringVector tokens = message.Split(' ', true);
	for (String& token : tokens)
	{
		if (token.StartsWith("%"))
		{
			if (token == "%FKEY")
				newMessage += Settings::GetAction(ActionType::FORWARD).binding->verboseName;
			else if (token == "%BKEY")
				newMessage += Settings::GetAction(ActionType::BACK).binding->verboseName;
			else if (token == "%LKEY")
				newMessage += Settings::GetAction(ActionType::LEFT).binding->verboseName;
			else if (token == "%RKEY")
				newMessage += Settings::GetAction(ActionType::RIGHT).binding->verboseName;
			else if (token == "%JKEY")
				newMessage += Settings::GetAction(ActionType::JUMP).binding->verboseName;
			else if (token == "%RVKEY")
				newMessage += Settings::GetAction(ActionType::REVIVE).binding->verboseName;
			else if (token == "%SKEY")
				newMessage += Settings::GetAction(ActionType::SLIDE).binding->verboseName;
		}
		else
		{
			newMessage += token + " ";
		}
	}
	return newMessage;
}

Sign::~Sign() {}
