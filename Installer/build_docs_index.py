# Builds the index that ships inside docs.7z.
#
# The pack is primarily for an agent, and an agent that has to extract 613 files
# to find out what is in them has already lost. The index gives it one small
# read: for every page, the title, the heading outline, and the settings the
# page documents - enough to choose the two or three pages worth extracting.
#
# The setting mapping is the point: an agent almost always arrives with a
# setting name, not a page name.
#
# Candidate names are *validated against SbieSettings.ini*, whose section names
# are the settings, rather than guessed from the text. Guessing does not work
# here - the docs write a setting three different ways (indented inside a code
# block, in backticks, in _italics_ in prose) and plenty of CamelCase words in
# those positions are not settings at all. Checking against the real list turns
# the whole problem into a lookup, and a page named after a setting is then
# matched even when its only mention is indented prose.
#
# Usage: build_docs_index.py <docs-dir> <settings-ini> <out-file> <relative-path>...
import io
import json
import os
import re
import sys

# Leading whitespace is allowed on purpose: the docs indent their ini examples,
# and anchoring hard to the line start misses most pages.
RE_SETTING_ASSIGN = re.compile(r"^[ \t]*([A-Za-z][A-Za-z0-9_]{2,})[ \t]*=", re.MULTILINE)
# The three other ways a setting gets written.
RE_CANDIDATE = re.compile(r"[`_]([A-Za-z][A-Za-z0-9_]{2,})[`_]")
RE_HEADING = re.compile(r"^(#{1,3})\s+(.+?)\s*#*$", re.MULTILINE)
RE_FRONTMATTER = re.compile(r"\A---\s*\n.*?\n---\s*\n", re.DOTALL)
RE_INI_SECTION = re.compile(r"^\[([A-Za-z][A-Za-z0-9_]*)\]\s*$", re.MULTILINE)


def read(path):
    with io.open(path, encoding="utf-8", errors="replace") as fh:
        return fh.read()


def known_settings(ini_path):
    """The authoritative setting names: the section names of SbieSettings.ini."""
    names = set()
    for name in RE_INI_SECTION.findall(read(ini_path)):
        # The file carries a few bookkeeping sections that are not settings;
        # they all start with an underscore.
        if not name.startswith("_"):
            names.add(name)
    return names


def page_entry(docs_dir, rel, settings_known):
    text = read(os.path.join(docs_dir, rel))
    body = RE_FRONTMATTER.sub("", text)

    headings = RE_HEADING.findall(body)
    title = None
    outline = []
    for hashes, heading in headings:
        heading = heading.strip()
        if len(hashes) == 1 and title is None:
            title = heading
        else:
            outline.append(heading)

    if not title:
        # No H1: fall back to the file name, which is how the site derives it too.
        title = os.path.splitext(os.path.basename(rel))[0]

    stem = os.path.splitext(os.path.basename(rel))[0]
    candidates = set(RE_SETTING_ASSIGN.findall(body))
    candidates.update(RE_CANDIDATE.findall(body))
    # One page per setting, named after it, is the convention in these docs - so
    # the file name is the single strongest candidate.
    candidates.add(stem)

    settings = sorted(candidates & settings_known)

    entry = {
        "path": rel.replace("\\", "/"),
        "title": title,
        "bytes": len(text.encode("utf-8")),
    }
    if outline:
        entry["outline"] = outline[:20]
    if settings:
        entry["settings"] = settings
    # The page that *is* the reference for a setting, as opposed to one that
    # merely mentions it. The agent should read this one first.
    if stem in settings_known:
        entry["documents"] = stem
    return entry


def main(argv):
    if len(argv) < 4:
        sys.stderr.write("usage: build_docs_index.py <docs-dir> <settings-ini> <out-file> <rel-path>...\n")
        return 2

    docs_dir, ini_path, out_file = argv[0], argv[1], argv[2]
    rels = argv[3:]

    # A single @listfile argument is also accepted, because that is what the
    # packing step already has to hand.
    if len(rels) == 1 and rels[0].startswith("@"):
        rels = [l.strip() for l in read(rels[0][1:]).splitlines() if l.strip()]

    settings_known = known_settings(ini_path)
    if not settings_known:
        sys.stderr.write("no settings found in %s - refusing to build a useless index\n" % ini_path)
        return 1

    pages = []
    by_setting = {}
    reference = {}
    for rel in rels:
        try:
            entry = page_entry(docs_dir, rel, settings_known)
        except (IOError, OSError) as err:
            sys.stderr.write("skipped %s: %s\n" % (rel, err))
            continue
        pages.append(entry)
        for name in entry.get("settings", []):
            by_setting.setdefault(name, []).append(entry["path"])
        if "documents" in entry:
            reference[entry["documents"]] = entry["path"]

    pages.sort(key=lambda e: e["path"])
    index = {
        "schema": 1,
        "language": "en",
        "pageCount": len(pages),
        "pages": pages,
        # The reverse mappings, built once here so the agent never has to scan.
        # settingIndex is every page that mentions a setting; settingPage is the
        # one page that is its reference.
        "settingIndex": {k: sorted(v) for k, v in sorted(by_setting.items())},
        "settingPage": dict(sorted(reference.items())),
    }

    with io.open(out_file, "w", encoding="utf-8", newline="\n") as fh:
        json.dump(index, fh, ensure_ascii=False, separators=(",", ":"), sort_keys=True)

    undocumented = len(settings_known) - len(reference)
    sys.stderr.write("indexed %d pages; %d of %d known settings have a reference page "
                     "(%d without), %d settings mentioned somewhere -> %s\n"
                     % (len(pages), len(reference), len(settings_known),
                        undocumented, len(by_setting), out_file))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
