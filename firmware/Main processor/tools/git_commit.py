#!/usr/bin/env python3
"""
git_commit.py - PlatformIO extra_script

Adds the short git commit as the GIT_COMMIT build flag. Falls back to
"unknown" when git is not available or the project is not a repository.

PlatformIO integration:
    extra_scripts = tools/git_commit.py
"""

import subprocess

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

flag = f'-DGIT_COMMIT=\\"{commit}\\"'
env.Append(CCFLAGS=[flag])
env.Append(CXXFLAGS=[flag])
