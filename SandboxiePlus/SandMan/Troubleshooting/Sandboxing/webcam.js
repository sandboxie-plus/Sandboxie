/*
*	group: sandboxing
*	class: sandboxing
* os_builds: 22000-99000
* name: Webcam or Sound does not work when sandboxed
* description: Description Text...
*
*/

let message = tr('To enable webcam support on Windows 11, the isolation level must be reduced. \n'+
'If you want to proceed, please press NEXT and select a sandbox to modify. \n');

if(!sbie.isCertValid())
  message += tr('\nPlease note that this required preset works only with a valid supporter certificate!');

wizard.showStatus(message, true);

let form = [
  {type: 'box', id: 'box'}
];

let data = wizard.showForm(form, tr('Select which box to turn into a reduced isolation app compartment box.'));

let boxName = data['box'];
let box = sbie.getBox(boxName);

box.setIniValue('NoSecurityIsolation', 'y');
box.appendIniValue('Template', 'OpenCOM');
box.setIniValue('DropAdminRights', 'y');
box.applyChanges();

{
  sbie.setupTrace();
  
  let form = [
      { id: 'yes', name: tr('Yes'), type: 'radio' },
      { id: 'no', name: tr('No'), type: 'radio' },
  ];
  let ret = wizard.showForm(form, tr('The mitigation has been applied please try out the web cam in %1 and indicate if the issue has been resolved.', boxName));
  if (ret['yes'] == true) {
      wizard.setResult(true);
  }
  if (ret['no'] == true) {
      wizard.setResult(false, 'Webcam mitigation not successfull');
      // todo roll back changes
  }
}
