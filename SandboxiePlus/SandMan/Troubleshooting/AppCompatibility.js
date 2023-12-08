/*
* group: system
* name: App compatibility checker
* description: This script checks which app compatibility templates need to be enabled
*
*/


result = [];

Classes = system.enumClasses();
Services = system.enumServices();
Products = system.enumProducts();
Objects = system.enumObjects();

templates = sbie.listTemplates();
//sbie.logMessage(tr('Loaded %1 templates', templates.length));

function checkTemplate(name)
{
  let template = sbie.getTemplate(name);

  //let title = template.getIniValue('Tmpl.Title');
  
  let scanScript = template.getIniValue('Tmpl.ScanScript');
  if(scanScript){
    let ret = false;
    try{
      ret = eval('(()=>{' + scanScript + '})()');
    } catch (error) {
      sbie.logMessage("error:" + error);
    }
    return ret;
  }
  
  let scan = template.getIniValue('Tmpl.Scan');
  let scanIpc = (scan.indexOf('i') != -1);
  let scanWin = (scan.indexOf('w') != -1);
  let scanSvc = (scan.indexOf('s') != -1);
  if (!(scanIpc || scanWin || scanSvc))
    return false;
    
  let settings = template.getIniSection();
  
  let keys = Object.keys(settings);
  for(let i=0; i < keys.length; i++)
  {
    let setting = keys[i];
    
    for(let j = 0; j < settings[setting].length; j++)
    {
      let value = settings[setting][j];
      
      if (scanIpc && (setting == "OpenIpcPath" || setting == "Tmpl.ScanIpc"))
      {
        if (value == "\\RPC Control\\epmapper")
          continue;
        if (value == "\\RPC Control\\OLE*")
          continue;
        if (value == "\\RPC Control\\LRPC*")
          continue;
        if (value == "*\\BaseNamedObjects*\\NamedBuffer*mAH*Process*API*")
          continue;

        if (system.checkObjects(value))
          return true;
      }
      else if (scanWin && (setting == "OpenWinClass" || setting == "Tmpl.ScanWinClass"))
      {
        // skip to unspecific entries
        if(value.substr(0,2) == "*:")
          continue;

        if (system.checkClasses(value))
          return true;
      }
      else if (scanSvc && setting == "Tmpl.ScanService")
      {
        if (system.checkServices(value))
          return true;
      }
      else if (scanSvc && setting == "Tmpl.ScanProduct")
      {
        if (system.checkProducts(value))
          return true;
      }
      else if (scanSvc && setting == "Tmpl.ScanKey")
      {
        if (system.checkRegKey(value))
          return true;
      }
      else if (scanSvc && setting == "Tmpl.ScanFile")
      {
        if (system.checkFile(system.expandPath(value)))
          return true;
      }
      //else if (scanUpd && setting == "Tmpl.ScanUpd")
      //{
      //  if (system.checkUpdates(value))
      //    return true;
      //}
    }
    
  }
    
  return false;
}


result = [];

for(let i=0; i < templates.length; i++)
{
  if(templates[i].substr(0,6) == "Local_")
    continue;
    
  if(checkTemplate(templates[i]))
    result.push(templates[i]);
}


// todo: detect conflicts
//sbie.logMessage(tr('No known conflicts detected'));