/*
*	group: other
*	class: sandboxing
* name: SBIE2224: Sandboxed program has crashed
* description: A Sandboxed program has crashed
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

let message = tr('The Process \'%1\' crashed', processName.toLowerCase());

// Known to run to in a green box
if(processName.toLowerCase() === "viber.exe"){

  message += '\n\n' + tr('This program is known to work in a Green (App Compartment) type sandbox.');

  invoke("SBIECONF");
  openOptions(message, boxName, 'General');

}
else {
  message += '\n\n' + tr('There is no known workaround for this process. Do you want to report this issue?');
  
  let form = [
    { id: 'bug', name: tr('Yes'), type: 'radio' },
    { id: 'no', name: tr('No'), type: 'radio' },
  ];

  let ret = wizard.showForm(form, message);
  if (ret['bug'] != true)
    wizard.setResult(true);
}



wizard.setResult(true);