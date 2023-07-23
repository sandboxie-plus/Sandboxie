/*
*	group: other
*	class: sandboxing
* name: SBIE1307: Program cannot access the Internet due to restrictions
* description: Program cannot access the Internet due to restrictions
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

let message = tr('Internet Access restrictions are in effect for the sandbox in which the program is running. The program is prohibited from accessing the Internet.');

/*if(processName){
  let form = [
    { id: 'yes', name: tr('Yes'), type: 'radio' },
    { id: 'no', name: tr('No'), type: 'radio' },
  ];
  
  message += '\n\n' + tr('Would you like to allow %1 to access the interent in future?', processName);
  let ret = wizard.showForm(form, message);
  if (ret['yes'] == true) {
      let box = sbie.getBox(boxName);
      // todo
      box.applyChanges();
  }
}
else*/ {
  //message += '\n\n' + tr('You can open the box option dialog to change this preset.');
  //wizard.showStatus(message, true);
  invoke("SBIECONF");
  openOptions(message, boxName, 'Internet');
}

wizard.setResult(true);