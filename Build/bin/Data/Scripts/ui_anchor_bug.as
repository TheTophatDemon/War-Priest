ui.root.RemoveAllChildren();
CreateUI();
Window@ window = ui.root.LoadChildXML(cache.GetResource("XMLFile", "UI/titlemenus/rebindWindow.xml"));
BorderImage@ panel = window.GetChild("panel");
for (int i = 0; i < 7; i++)
{
	Button@ butt = panel.LoadChildXML(cache.GetResource("XMLFile", "UI/titlemenus/rebindButton.xml"));
	butt.SetPosition(butt.position.x, butt.position.y + i * butt.height);
	//These lines accomplish nothing more
	//butt.SetMaxAnchor(0.5, 0.0);
	//butt.SetMinAnchor(0.5, 0.0);
}