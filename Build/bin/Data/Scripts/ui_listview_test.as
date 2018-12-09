ui.root.RemoveAllChildren();
CreateUI();
XMLFile@ styleFile = cache.GetResource("XMLFile", "UI/DefaultStyle.xml");
UIElement@ window = ui.root.LoadChildXML(cache.GetResource("XMLFile", "UI/titlemenus/rebindWindow.xml"), styleFile);
ListView@ listView = cast<ListView@>(window.GetChild("panel", true));
listView.layoutMode = LM_FREE;
listView.layoutBorder = IntRect(6, 6, 6, 6);
listView.clipChildren = false;
listView.horizontalScrollBar.clipChildren = true;
listView.horizontalScrollBar.clipBorder = IntRect(200, 200, 200, 200);
for (int i = 0; i < 50; i++)
{
	UIElement@ parent = UIElement();
	UIElement@ button = parent.LoadChildXML(cache.GetResource("XMLFile", "UI/titlemenus/rebindButton.xml"), styleFile);
	button.focusMode = FM_NOTFOCUSABLE;
	parent.SetAlignment(HA_CENTER, VA_TOP);
	parent.SetSize(button.width, button.height);
	parent.SetMinSize(parent.width, parent.height);
	parent.SetMaxSize(parent.width, parent.height);
	/*Text@ text = cast<Text@>(parent.CreateChild("Text"));
	text.text = "I Dare Say What the Devil Did You Just Audaciously Proclaim About My Well-Being, You Trollop!?";
	text.SetStyle("Text", styleFile);*/
	listView.AddItem(parent);
}