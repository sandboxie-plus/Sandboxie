/*
*	group: other
*	class: sandboxing
* name: SBIE2189: Application is likely chromium based
* description: Application is likely chromium based but its image type is not set to chrome
*
*/

if (typeof boxName === 'undefined' || !boxName) {
  let form = [
      { id: 'box', name: tr('Select affected sandbox'), type: 'box' },
      { id: 'process', name: tr('Enter Process Name'), type: 'edit' },
  ];
  let ret = wizard.showForm(form);
  boxName = ret.box;
  processName = ret.process;
}

if (typeof msgData === 'undefined' || !msgData[0]) {
  msgData = [processName, processName];
}

let message = tr('The Process "%1" is likely chromium based but its image type is not set to chrome, it is likely to fail. Do you want to set its ImageType setting to chrome for this box?', processName.toLowerCase());

// Known Fix set Image Type
  
  let form = [
    { id: 'yes', name: tr('Yes, add "%1" to Sandboxie.ini', 'SpecialImage=chrome,' + msgData[0]), type: 'radio' , value: true},
    { id: 'no', name: tr('No, report application to sbie devs'), type: 'radio' },
    //{ id: 'report', name: tr('Report application to sbie devs'), type: 'check' },
  ];

  let ret = wizard.showForm(form, message);
  if (ret['yes'] == true)
  {
    let box = sbie.getBox(boxName);
    box.appendIniValue('SpecialImage','chrome,' + msgData[0]);
    box.applyChanges();
  }
  
  //if(ret['report'] == true)
  if (ret['no'] == true)
    wizard.setResult(false, 'The Process "' + msgData[1] + '" is likely chromium based but the SpecialImage is none.');
  else
    wizard.setResult(true);

