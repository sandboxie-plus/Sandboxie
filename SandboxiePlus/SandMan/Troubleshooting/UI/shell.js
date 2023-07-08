/*
*	group: ui
*	class: ui
* name: Explorer Context Menu extension does not work
* description: This procedure will re install the shell integration
*
*/

let form = [
    {type: 'check', name: tr('Add Run Sandboxed'), value: true, disabled: true},
    {type: 'check', id: 'runUnBoxed', name: tr('Add Run Unsandboxed')}
];

let sys_version = system.version();
if(sys_version.major >= 11) {
  form.push({type: 'check', id: 'legacy', name: tr('Install legacy shell extensions')});
}

let obj = wizard.showForm(form, tr('Select which shell options should be reinstalled'));

sbie.shellRemove();
sbie.shellInstall(obj);

// todo query user if mitigation was successful
wizard.setResult(true);