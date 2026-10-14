# Korean Localization Review and Maintenance

This fork is used to review and improve the Korean localization of Sandboxie Plus while keeping the original application's functionality and security model unchanged.

## Why the review is needed

Some Korean strings use literal English word order, inconsistent terminology, or descriptions that can be misunderstood when users configure isolation and security settings. This matters most for access rules, administrative privileges, encrypted containers, file recovery, and warnings about data loss. The purpose of this work is to make the interface easier to understand for Korean Windows users without weakening or exaggerating the meaning of the original English messages.

I am checking both short interface labels and longer messages against the English source. Accurate existing translations are kept, and corrections are limited to wording, mistakes, consistency, or missing translations.

## Translation scope

- `SandboxiePlus/SandMan/sandman_ko.ts` — Sandboxie Plus interface, dialog boxes, notifications, settings, and warnings.
- `SandboxiePlus/SandMan/Troubleshooting/lang_ko.json` — troubleshooting steps and SBIE messages.
- `Installer/Languages.iss` — installer and uninstaller messages.
- `Sandboxie/msgs/Text-Korean-1042.txt` — Classic/core diagnostic messages; reviewed separately from the Plus interface.
- `Installer/isl/Korean.isl` — third-party Inno Setup messages; original attribution and email addresses stay unchanged.

## Translation guidelines

1. Write naturally in Korean, especially for menus and standard Windows dialogs, without inventing functionality that does not exist.
2. Use consistent terms. Prefer `샌드박스` for sandbox, `박스` for a named box, `격리` for isolation, and `접근` for access when it reads naturally.
3. Preserve technical identifiers, registry paths, file paths, policy keywords, and names such as `DropAdminRights`, `FakeAdminRights`, `ImageType`, and SBIE message codes.
4. Preserve `%1`, `%2`, `%n` and other placeholders, links, HTML/XML formatting, keyboard accelerators, installer macros, and JSON keys.
5. Treat data-loss, encryption, and reduced-isolation warnings as security-sensitive content. Translate the complete meaning, not a shortened paraphrase.
6. Avoid replacing accurate translations merely for stylistic uniformity.

## Credits and reference

**Korean localization maintenance:** M26Pershing90mm (Sakai).

The earlier translators' historical contributions, copyright notices, and existing email addresses remain unchanged. The Inno Setup Korean translator credit is not reassigned.

This is an independent Korean localization review. Pull request [#5602](https://github.com/sandboxie-plus/Sandboxie/pull/5602) is not used as the reference translation, and its changes are not being resubmitted as an independent correction.

## Validation checklist

- Confirm the Qt TS file parses as XML, and the troubleshooting JSON parses correctly.
- Compare source and translation placeholder sets.
- Check rich-text tags, links, original technical identifiers, and accelerators.
- Review currently empty entries and look for misleading security messages.
- Build the Qt translation and test real GUI screens.
- Test installer/uninstaller screens before submitting upstream.
- Review the complete diff for unintended changes.

Static checks are not equivalent to full application testing. Any untested changes should be described accurately.
