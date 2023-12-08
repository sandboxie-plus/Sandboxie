/*
*	group: other
*	class: sandboxing
*   name: SBIEMSG, I'm getting a cryptic SBIExxxx message
*   description: Handle all sbie messages...
*
*/

if (typeof msgCode === 'undefined') {
  let form = [
    { id: 'code', name: tr('Sbie Message Code'), type: 'edit' },
    { id: 'box', name: tr('Select affected sandbox'), type: 'box' },
    { id: 'process', name: tr('Name affected process'), type: 'edit', hint: 'Program.exe when applicable' },
  ];
  let ret = wizard.showForm(form, tr('Please enter the SBIEMSG ID you have encountered.'));
  msgCode = ret.code;
  if(msgCode.substr(0,4).toUpperCase() == "SBIE") msgCode = msgCode.substr(4);
  docLink = 'https://sandboxie-plus.com/go.php?to=sbie-sbie' + msgCode;
  msgData = [];
  //sbieMsg = sbie.formatMessage(msgCode, msgData);
  boxName = ret.box;
  processName = ret.process;
}

let res = invoke("SBIE" + msgCode);
if(res === false) {

  let message = tr('There is no automated troubleshooting available for SBIE%1 yet.<br />', msgCode);
  if (typeof sbieMsg !== 'undefined')
    message += tr('The full message text is: %1<br />', sbieMsg);
  
  message += tr('Visit out <a href="%1">online-documentation</a> to learn more about this, and other sbie messages.', docLink);
  message += '<br /><br />';
  message += tr('Would you like to collect some logs to help us resolve the issue.');

  let form = [
    { id: 'yes', name: tr('Yes'), type: 'radio' },
    { id: 'no', name: tr('No'), type: 'radio' },
  ];
  let ret = wizard.showForm(form, message);
  system.log('yes: ' + ret['yes']);
  system.log('no: ' + ret['no']);
  if (ret['yes'] == true) {
      
    sbie.setupTrace();

    wizard.showStatus(tr('Now please run your application again to trigger the Sbie message.<br /><br />') + 
                      tr('Press NEXT when you are done collecting logs.'), true);
  }

  wizard.reportAdd('MsgCode', msgCode);
  if(msgData) wizard.reportAdd('MsgData', msgData);
  if(processName) wizard.reportAdd('ProcessName', processName);
  
}