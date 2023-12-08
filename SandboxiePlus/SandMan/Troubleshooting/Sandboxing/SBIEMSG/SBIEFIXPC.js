/*
*	group: library
*	class: sandboxing
* name: fix parental controls breaking sandboxie
* description: stops AppId driver and AppIdSvc service and disables the offending policy file
*
*/

function try_fix_PC_SRP()
{
  let form = [
    {id: 'stop', name: 'Stop AppIdSvc service and AppId driver (temporary fix)', type: 'radio'},
    {id: 'disable', name: 'Disable Parental Control Policy', type: 'radio'},
    {id: 'failed', name: 'The above mitigations did not work', type: 'radio'},
  ];
  let obj = wizard.showForm(form, tr('It seems you are using a non-administrative user account on a system with enabled parental controls, this is known to be incompatible with Sandboxie.\nYou have the following options to resolve the issue.'));
  
  //_debugger();
  
  if(obj.failed){
    wizard.setResult(false, 'Available mitigations failed');
    return;
  }
  
  let command = '/c net stop AppIdSvc & net stop AppId';
  if(obj.disable)
    command += ' & move %systemroot%\\system32\\AppLocker\\Plugin.133422605925491691.Policy %systemroot%\\system32\\AppLocker\\Disabled__Plugin.133422605925491691.Policy__'
  
  let ret = system.execute('cmd.exe', command, {elevate: true});

  if(ret.error)
    wizard.setResult(false, 'Failed to execute recured command: ' + ret.error);
  else
    wizard.setResult(true);
}