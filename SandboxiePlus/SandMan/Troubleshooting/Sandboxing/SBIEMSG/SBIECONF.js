/*
*	group: library
*	class: sandboxing
* name: open config
* description: opens box config on a given page
*
*/

function openOptions(message, box, page)
{
  message += '\n\n' + tr('Do you want to open the box option dialog to change this preset?');
  
  let form = [
    { id: 'yes', name: tr('Yes'), type: 'radio' },
    { id: 'no', name: tr('No, it is fine as it is'), type: 'radio' },
    { id: 'bug', name: tr('No, but I want to report a bug'), type: 'radio' },
  ];

  let ret = wizard.showForm(form, message);
  if (ret['yes'] == true) {
    if (!box) {
      let form = [
        { id: 'box', name: tr('Select affected sandbox'), type: 'box' },
      ];
      let ret = wizard.showForm(form);
      box = ret.box;
    }
    wizard.openOptions(box, page);
  } 
  if (ret['bug'] != true)
    wizard.setResult(true);
}