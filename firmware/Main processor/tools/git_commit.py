#!/usr/bin/env python3
"""
git_commit.py - PlatformIO extra_script

Writes include/GitCommit.h with the short git commit. Falls back to "unknown"
when git is not available or the project is not a repository.

PlatformIO integration:
    extra_scripts = tools/git_commit.py
"""

import subprocess
from pathlib import Path

Import("env")

try:
    commit = subprocess.check_output(
        ["git", "rev-parse", "--short", "HEAD"],
        cwd=env.subst("$PROJECT_DIR"),
        text=True,
        stderr=subprocess.DEVNULL,
    ).strip()
except (OSError, subprocess.CalledProcessError):
    commit = "unknown"

output = Path(env.subst("$PROJECT_DIR")) / "include" / "GitCommit.h"
output.write_text(
    "/*\n"
    " *\n"
    " * Copyright Marc Ketel\n"
    " * SPDX-License-Identifier: Apache-2.0\n"
    " *\n"
    " */\n"
    "\n"
    "#pragma once\n"
    "\n"
    f'#define GIT_COMMIT "{commit}"\n',
    encoding="utf-8",
)
