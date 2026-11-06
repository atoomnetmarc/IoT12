#!/usr/bin/env python3
"""
generate_webui_header.py - PlatformIO extra_script

Reads webui/index.html, style.css and app.js, injects a cache-busting
timestamp, gzips each file and writes include/WebContent.h.

PlatformIO integration:
    extra_scripts = tools/generate_webui_header.py

Manual run:
    python3 tools/generate_webui_header.py
"""

import gzip
import os
import time
from pathlib import Path


def _project_root():
    return Path(os.environ.get("PROJECT_DIR", os.getcwd()))


def _cache_bust_token():
    """Unix timestamp string for cache-busting."""
    return str(int(time.time()))


def _c_array(lines, name, blob):
    lines.append(f"static const uint8_t {name}[] PROGMEM = {{")
    for i in range(0, len(blob), 16):
        chunk = ", ".join(f"0x{b:02x}" for b in blob[i : i + 16])
        lines.append(f"    {chunk},")
    lines.append("};")
    lines.append("")


def generate():
    """Generate WebContent.h from the webui assets."""
    project_root = _project_root()
    webui = project_root / "webui"
    output_file = project_root / "include" / "WebContent.h"

    cache_bust = _cache_bust_token()
    html = (webui / "index.html").read_text(encoding="utf-8")
    css = (webui / "style.css").read_text(encoding="utf-8")
    js = (webui / "app.js").read_text(encoding="utf-8")
    html = html.replace("{{CACHE_BUST}}", cache_bust)

    html_gz = gzip.compress(html.encode("utf-8"))
    css_gz = gzip.compress(css.encode("utf-8"))
    js_gz = gzip.compress(js.encode("utf-8"))

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
        f"static const uint32_t WEBUI_HTML_GZ_LEN = {len(html_gz)};",
        f"static const uint32_t WEBUI_CSS_GZ_LEN = {len(css_gz)};",
        f"static const uint32_t WEBUI_JS_GZ_LEN = {len(js_gz)};",
        "",
    ]

    _c_array(lines, "WEBUI_HTML_GZ", html_gz)
    _c_array(lines, "WEBUI_CSS_GZ", css_gz)
    _c_array(lines, "WEBUI_JS_GZ", js_gz)

    output_file.write_text("\n".join(lines), encoding="utf-8")
    print(
        f"WebContent.h generated (HTML {len(html_gz)}, CSS {len(css_gz)}, "
        f"JS {len(js_gz)} bytes gzipped)"
    )


generate()
