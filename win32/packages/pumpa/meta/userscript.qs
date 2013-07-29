function Component()
{
  var programFiles = installer.environmentVariable("ProgramFiles");
  if (programFiles != "")
    installer.setValue("TargetDir", programFiles + "/pumpa");
}

Component.prototype.isDefault = function()
{
 // select the component by default
 return true;
}

Component.prototype.createOperations = function()
{
    // call default implementation to actually install README.txt!
    component.createOperations();

    if (installer.value("os") === "win") {
        component.addOperation("CreateShortcut", "@TargetDir@/pumpa.exe", "@StartMenuDir@/pumpa.lnk");
    }
}