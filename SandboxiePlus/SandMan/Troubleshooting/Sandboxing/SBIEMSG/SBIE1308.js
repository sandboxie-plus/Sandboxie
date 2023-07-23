/*
*	group: other
*	class: sandboxing
* name: SBIE1308: Program cannot start due to restrictions
* description: Program cannot start due to restrictions
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

let message = tr('Start/Run restrictions are in effect for the sandbox in which the program is running. The program is prohibited from starting or running.');

/*if(processName){
  let form = [
    { id: 'yes', name: tr('Yes'), type: 'radio' },
    { id: 'no', name: tr('No'), type: 'radio' },
  ];

  message += '\n\n' + tr('Would you like to allow %1 to start in this sandbox?', processName);
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
  openOptions(message, boxName, 'Start');
}

wizard.setResult(true);