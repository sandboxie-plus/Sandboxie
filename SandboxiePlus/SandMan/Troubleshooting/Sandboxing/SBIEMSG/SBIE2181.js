/*
*	group: other
*	class: sandboxing
* name: SBIE2181: Failed to load SbieDll.dll
* description: Fix the DACLs of sandboxies home folder
*
*/

wizard.showStatus(tr('Failures to load SbieDll.dll when encountered by chrome, or an other software employing app containers, ' +
 'is often caused by invalid DACL entries for the sandboxie home directory. ' + 
 'This mitigation measure will fix these, for this it will prompt for admin privileges for kmdutill.exe which need to be granted.'), true);

let ret = system.execute(system.expandPath('%SbieHome%\\kmdutil.exe'), 'fixdacls', {elevate: true});

if(ret.error)
  wizard.setResult(false, 'Failed to start kmdutill, error: ' + ret.error);
else if(ret.exitCode != 0)
  wizard.setResult(false, 'kmdutill returned an error: ' + ret.exitCode);
else
{
  sbie.setupTrace();
  
  let form = [
      { id: 'yes', name: tr('Yes'), type: 'radio' },
      { id: 'no', name: tr('No'), type: 'radio' },
  ];
  let ret = wizard.showForm(form, tr('The DACLs have been adjusted please try to run your application again and indicate if the issue has been resolved.'));
  if (ret['yes'] == true) {
      wizard.setResult(true);
  }
  if (ret['no'] == true) {
      wizard.setResult(false, 'DACLs fix did not resolve the issue');
  }
}
