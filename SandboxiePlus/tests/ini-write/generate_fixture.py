"""Extract the production mutation and options-save paths for fault injection."""
import argparse
import re
from pathlib import Path


def extract(text: str, signature: str) -> str:
    matches = list(re.finditer(r'^' + signature + r'[^\n]*\n\{.*?^\}', text, re.M | re.S))
    if len(matches) != 1:
        raise ValueError(f'Expected one production function: {signature}')
    return matches[0].group(0)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('output', type=Path)
    parser.add_argument('--source-root', type=Path, default=Path(__file__).resolve().parents[2])
    args = parser.parse_args()
    ini = (args.source_root / 'QSbieAPI/Sandboxie/SbieIni.cpp').read_text(encoding='utf-8-sig')
    options = (args.source_root / 'SandMan/Windows/OptionsWindow.cpp').read_text(encoding='utf-8-sig')
    save = extract(options, r'(?:void|bool) COptionsWindow::SaveConfig\(\)')
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / 'fixture_return.inc').write_text(
        '#define SAVE_CONFIG_RESULT ' + save.split()[0] + '\n', encoding='utf-8')
    chunks = [
        extract(ini, re.escape('SB_STATUS CSbieIni::SetBoolSafe(')),
        extract(ini, re.escape('SB_STATUS CSbieIni::UpdateTextList(')),
        extract(options, re.escape('void COptionsWindow::WriteAdvancedCheck(QCheckBox* pCheck, const QString& Name, const QString& OnValue')),
        extract(options, re.escape('void COptionsWindow::WriteTextList(')),
        save,
        extract(options, re.escape('bool COptionsWindow::apply(')),
        extract(options, re.escape('void COptionsWindow::ok(')),
    ]
    (args.output / 'ini_write_under_test.inc').write_text('\n\n'.join(chunks) + '\n', encoding='utf-8')


if __name__ == '__main__':
    main()
