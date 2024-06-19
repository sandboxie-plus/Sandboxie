/*
*	group: sandboxing
*	class: sandboxing
* name: Low FPS in sandboxed games
* description: This procedure will optimize the box settings for gaming
*
*/

let form = [
  {type: 'box', id: 'box'}
];

let data = wizard.showForm(form, tr('Select which box to optimize.'));

let boxName = data['box'];
let box = sbie.getBox(boxName);

let message = tr('To apply recommended settings press NEXT. \n');

if(!sbie.testFeature('SMod'))
  message += tr('\nPlease note that this required preset works only with a valid supporter certificate!');

wizard.showStatus(message, true);

box.setIniValue('SysCallLockDown', 'y');
box.applyChanges();
box.start();

{
  sbie.setupTrace();
  
  let form = [
      { id: 'yes', name: tr('Yes'), type: 'radio' },
      { id: 'no', name: tr('No'), type: 'radio' },
  ];
  let ret = wizard.showForm(form, tr('The options has been applied please tryout your game in %1 and indicate if the issue has been resolved.', boxName));
  if (ret['yes'] == true) {
      wizard.setResult(true);
  }
  if (ret['no'] == true) {
      wizard.setResult(false, tr('FPS optimization not successful'));
      // todo roll back changes
  }
}
