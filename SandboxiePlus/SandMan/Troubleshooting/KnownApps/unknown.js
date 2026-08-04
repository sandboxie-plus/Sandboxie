/*
*	group: apps
*	class: sandboxing
* name: Unknown application crashed
* description: An Unknown application crashed
* bold: true
*
*/

  message = tr('Do you want to report this issue?');
  
  let form = [
    { id: 'bug', name: tr('Yes'), type: 'radio' },
    { id: 'no', name: tr('No'), type: 'radio' },
  ];

  let ret = wizard.showForm(form, message);
  if (ret['bug'] != true)
    wizard.setResult(true);