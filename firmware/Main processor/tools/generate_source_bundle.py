#!/usr/bin/env python3
"""
generate_source_bundle.py - PlatformIO extra_script

Packs the main processor and heat controller source trees into one gzipped tar
archive and writes include/SourceBundle.h. The web server serves the archive so
the source of the running firmware is downloadable from the webui.

PlatformIO integration:
    extra_scripts = tools/generate_source_bundle.py

Manual run:
    python3 tools/generate_source_bundle.py
"""

import gzip
import io
import os
import tarfile
from pathlib import Path

# Files and directories that belong to the source drop.
MAIN_ENTRIES = [
    "src",
    "include",
    "lib",
    "webui",
    "tools",
    "platformio.ini",
    "README.md",
    "LICENSE",
    ".clang-format",
]
HEATER_ENTRIES = [
    "src",
    "include",
    "lib",
    "platformio.ini",
    "README.md",
    "LICENSE",
    "MEASUREMENTS.md",
    ".clang-format",
]

# Generated headers must not end up in the archive: they contain the archive.
EXCLUDED_NAMES = {"WebContent.h", "SourceBundle.h"}
EXCLUDED_DIRS = {".pio", ".git", "__pycache__", "node_modules"}


def _project_root():
    return Path(os.environ.get("PROJECT_DIR", os.getcwd()))


def _add_file(archive, path, name):
    """Add one file with a stable name, owner and timestamp."""
    info = archive.gettarinfo(str(path), arcname=name)
    info.mtime = 0
    info.uid = 0
    info.gid = 0
    info.uname = ""
    info.gname = ""
    info.mode = 0o644
    with path.open("rb") as handle:
        archive.addfile(info, handle)


def _add_entry(archive, root, entry, prefix):
    """Add one file or directory tree under the given archive prefix."""
    path = root / entry
    if not path.exists():
        return

    if path.is_file():
        _add_file(archive, path, f"{prefix}/{entry}")
        return

    for item in sorted(path.rglob("*")):
        if not item.is_file():
            continue
        if any(part in EXCLUDED_DIRS for part in item.relative_to(root).parts):
            continue
        if item.name in EXCLUDED_NAMES:
            continue
        _add_file(archive, item, f"{prefix}/{item.relative_to(root).as_posix()}")


def _gzip(data):
    """Return deterministic gzip output."""
    out = io.BytesIO()
    with gzip.GzipFile(fileobj=out, mode="wb", compresslevel=9, mtime=0) as handle:
        handle.write(data)
    return out.getvalue()


def _bundle(project_root):
    """Return one gzipped tar with both source trees."""
    raw = io.BytesIO()
    with tarfile.open(fileobj=raw, mode="w", format=tarfile.GNU_FORMAT) as archive:
        for entry in MAIN_ENTRIES:
            _add_entry(archive, project_root, entry, "Main processor")

        heater_root = project_root.parent / "Heat controller"
        if heater_root.is_dir():
            for entry in HEATER_ENTRIES:
                _add_entry(archive, heater_root, entry, "Heat controller")

    return _gzip(raw.getvalue())


def _c_array(lines, name, blob):
    lines.append(f"static const uint8_t {name}[] PROGMEM = {{")
    for i in range(0, len(blob), 16):
        chunk = ", ".join(f"0x{b:02x}" for b in blob[i : i + 16])
        lines.append(f"    {chunk},")
    lines.append("};")
    lines.append("")


def generate():
    """Generate SourceBundle.h from the two source trees."""
    project_root = _project_root()
    output_file = project_root / "include" / "SourceBundle.h"

    source_gz = _bundle(project_root)

    lines = [
        "/*",
        " *",
        " * Copyright Marc Ketel",
        " * SPDX-License-Identifier: Apache-2.0",
        " *",
        " */",
        "",
        "#pragma once",
        "",
        "#include <pgmspace.h>",
        "",
        f"static const uint32_t SOURCE_GZ_LEN = {len(source_gz)};",
        "",
    ]

    _c_array(lines, "SOURCE_GZ", source_gz)

    output_file.write_text("\n".join(lines), encoding="utf-8")
    print(f"SourceBundle.h generated ({len(source_gz)} bytes gzipped)")


generate()
