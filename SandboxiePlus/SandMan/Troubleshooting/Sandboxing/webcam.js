/*
*	group: sandboxing
*	class: sandboxing
* _os_builds: 22000-99000
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

let box = sbie.getBox(data['box']);

box.setIniValue('NoSecurityIsolation', 'y');
box.appendIniValue('Template', 'OpenCOM');
box.setIniValue('DropAdminRights', 'y');
box.applyChanges();