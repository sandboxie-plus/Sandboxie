/*
*	group: browser
*	class: sandboxing
* name: Extensions or Credentials are disappearing
* description: Web Browser Extensions are being reset, or login credentials are forgotten
*
*/

let form = [
  {type: 'box', id: 'box'}
];

let data = wizard.showForm(form, tr('Select which box to fix.'));

let boxName = data['box'];
let box = sbie.getBox(boxName);

let message = tr('To apply recommended settings press NEXT. \n');

wizard.showStatus(message, true);

box.appendIniValue('Template', 'Chromium_Elevation');
box.applyChanges();
box.start("default_browser");

{
  sbie.setupTrace();
  
  let form = [
      { id: 'yes', name: tr('Yes'), type: 'radio' },
      { id: 'no', name: tr('No'), type: 'radio' },
  ];
  let ret = wizard.showForm(form, tr('The options have been applied. Please test your browser in %1 and indicate if the issue has been resolved.', boxName));
  if (ret['yes'] == true) {
      wizard.setResult(true);
  }
  if (ret['no'] == true) {
      wizard.setResult(false, tr('Chrome elevation service fix not successful'));
      // todo roll back changes
  }
}
