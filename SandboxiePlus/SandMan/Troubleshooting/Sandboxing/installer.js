/*
*	group: sandboxing
*	class: sandboxing
* name: Failed to install application into a sandbox
* description: Description Text...
*
*/

let form = [
    { id: 'path', name: tr('Enter Installer Path'), type: 'file' },
    { id: 'box', name: tr('Select a sandbox to install into'), type: 'box' },
];
let ret = wizard.showForm(form);

wizard.reportAdd('installer', ret['path']);

let box = sbie.getBox(ret['box']);

// SysCallLockDown, UseSecurityMode

if(box.getIniValue('DropAdminRights', {type: 'bool'}) || box.getIniValue('UseSecurityMode', {type: 'bool'}))
{
  let fake = box.getIniValue('FakeAdminRights', {type: 'bool'});
  
  let prompt2 = tr('This box has DropAdminRights enabled, preventing execution of installers. Do you want to disable this restriction, that will reduce the security level.');
  
  let form2 = [{ type: 'radio', id: 'yes', name: tr('Disable DropAdminRights') }];
  if(!fake) form2.push({ type: 'radio', id: 'no', name: tr('Enable FakeAdminRights') })
  let ret2 = wizard.showForm(form2, prompt2);
  
  if(ret2['yes']) {
    if(box.getIniValue('DropAdminRights', {type: 'bool'}))
      box.delIniValue('DropAdminRights');
    else if(box.getIniValue('UseSecurityMode', {type: 'bool'}))
      box.delIniValue('UseSecurityMode');
  } else if(ret2['no'])
    box.setIniValue('FakeAdminRights', 'y');
}

box.setIniValue('MsiInstallerExemptions', 'y');
//box.setIniValue('ApplyElevateCreateProcessFix', 'y');

let pid = box.startTask(ret['path']);
wizard.showStatus(tr('running installer, pid: %1 press NEXT once it finishes to continue', pid), true);
box.terminate();

let form3 = [
    { type: 'radio', id: 'yes', name: tr('Yes') },
    { type: 'radio', id: 'no', name: tr('No') },
];
let ret3 = wizard.showForm(form3, tr('Was the issue resolved?'));
if (ret3['yes'] == true)
    wizard.setResult(true);
else
    wizard.setResult(false, 'no mitigation worked');