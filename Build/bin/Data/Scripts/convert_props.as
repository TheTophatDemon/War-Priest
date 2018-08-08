Print("Converting props...");
Array<Node@>@ ents = scene.GetChildren(true);
Print(ents.length);
for (int i = 0; i < ents.length; i++)
{
	if (ents[i].HasTag("statue") || ents[i].HasTag("lift")) continue;
	if (ents[i].name == "bonus")
	{
		StaticModel@ sm = ents[i].GetComponent("StaticModel");
		sm.model = cache.GetResource("Model", "Models/bonus.mdl");
		sm.material = cache.GetResource("Material", "Materials/skins/bonus_skin.xml");
		Print("Set bonus material");
	}
	if (ents[i].name == "model" && ents[i].parent.name == "player")
	{
		AnimatedModel@ am = ents[i].GetComponent("AnimatedModel");
		am.material = cache.GetResource("Material", "Materials/skins/grungle_skin.xml");
		Print("Set player material");
	}
	if (ents[i].parent.name == "map")
	{
		Print("Reparenting " + ents[i].name + "...");
		ents[i].parent = scene;
	}
	if (ents[i].HasTag("propInstancer"))
	{
		Print("Deleting propInstancer " + ents[i].name + "...");
		Array<Node@>@ children = ents[i].GetChildren(true);
		for (int j = 0; j < children.length; j++)
		{
			Print("-Reparenting " + children[j].name);
			children[j].parent = scene;
		}
		ents[i].Remove();
	}
	if (ents[i].HasComponent("StaticModel"))
	{
		StaticModel@ sm = ents[i].GetComponent("StaticModel");
		if (sm.model != null)
		{
			if (sm.model.name.Contains("/props/"))
			{
				String[] path = sm.model.name.Split('/');
				String propName = path[path.length - 1].Replaced(".mdl", "");
				Print("Converting " + sm.model.name + " (" + propName + ")");
				if (!ents[i].HasTag("prop") && !ents[i].HasTag("lift") && !ents[i].HasTag("statue")) 
				{
					ents[i].AddTag("prop");
					Print("-Added prop tag");
					if (propName != "skeleton_leaning" && propName != "skeleton_lying"
						&& propName != "skeleton_upper" && propName != "skull")
					{
						ents[i].vars["noCollision"] = Variant(true);
						Print("-Disabled collisions");
					}
				}
				if (ents[i].name != propName)
				{
					ents[i].name = propName;
					Print("-Set name to " + propName);
				}
				cache.GetResource("XMLFile", "Objects/prop_" + ents[i].name + ".xml");
			}
		}
		else
		{
			Print(ents[i].name + " has no model???");
		}
	}
}
Print("Props converted.");