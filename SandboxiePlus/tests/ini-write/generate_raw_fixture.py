"""Extract raw INI save callers from both production dialogs for fault injection."""
import argparse
import re
from pathlib import Path

from generate_fixture import extract


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument('output', type=Path)
    parser.add_argument('--source-root', type=Path, default=Path(__file__).resolve().parents[2])
    args = parser.parse_args()
    chunks = []
    declarations = []
    for name in ('OptionsWindow', 'SettingsWindow'):
        text = (args.source_root / 'SandMan/Windows' / (name + '.cpp')).read_text(encoding='utf-8-sig')
        for method in ('SaveIniSection', 'OnSaveIni', 'apply', 'ok', 'SetIniEdit', 'OnIniChanged', 'OnCancelEdit'):
            function = extract(text, r'(?:void|bool) ' + re.escape('C' + name + '::' + method + '('))
            chunks.append(function)
            if method in ('SaveIniSection', 'apply'):
                declarations.append('#define ' + name.upper() + '_' + method.upper() + '_RESULT ' + function.split()[0])
    options = (args.source_root / "SandMan/Windows/OptionsWindow.cpp").read_text(encoding="utf-8-sig")
    chunks.append(extract(options, re.escape("void COptionsWindow::OnTab(QWidget*")))
    settings = (args.source_root / "SandMan/Windows/SettingsWindow.cpp").read_text(encoding="utf-8-sig")
    chunks.append(extract(settings, re.escape("void CSettingsWindow::OnTab(QWidget*")))
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / 'raw_ini_under_test.inc').write_text('\n\n'.join(chunks) + '\n', encoding='utf-8')
    (args.output / 'raw_ini_return.inc').write_text('\n'.join(declarations) + '\n', encoding='utf-8')


if __name__ == '__main__':
    main()
