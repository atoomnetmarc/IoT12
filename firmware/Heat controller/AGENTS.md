# Agent Rules Index

## Documentation Map

- **README.md** — General project description.

## Core Rules

- **Hobby-grade project** - Be gentle with advice. Be honest if the code smells.
- **Update docs on code change** - Check any `.md` file when the code changes. README.md and the docs/ rule files must stay in sync with the code.
- **Language** - Write everything in English. Reply in the user's language.
- **No doxing** - Never document absolute paths or real network names.
- **Prove calculations** - Validate calculations with GCC or Python. Do not reason manually.
- **X-Y problem?** - Ask the user about X. Do not propose a solution Y first.
- **Ask questions** - Asking the user questions is allowed and encouraged. Prefer the `/grill-me` skill when it is available. Present ALL questions with multiple answer options. Use a table with one row per option and these columns: `Answer | Industry standard | Risk | Reward | Fits test-framework | Notes`. Read the relevant docs or code before you ask. Add a final row labeled "From scratch" to propose a custom alternative. After the table, state the recommended answer and explain why.
- **Answer questions, execute commands** - Answer a question and offer follow-up choices when the user asks one. Act only when the user gives an explicit instruction.
- **Concise completions** - State only the final result in `attempt_completion`. Do not re-explain steps, summarize previous outputs, or repeat information the user already saw.
- **No fancy UTF-8 characters** - Use a simple ASCII character when one exists.

## Code Quality Rules

- ASD-STE100
- Readability over cleverness.
- Understand fully before simplifying.
- YAGNI. Do not speculate.
- Use native functionality.
- No dependencies for code that a few lines do.
- DRY.
- Boring over clever.
- One line if possible.
- No scaffolding for later.
- No dead code.
- Deletion over addition.
- Smallest diff.

## Quick Reference

All detailed coding rules are in this file. Key reminders:

- Detect and match existing file coding style
- Run `/usr/bin/clang-format` (project `.clang-format`) on new/modified `.cpp` and `.h` files when done editing
- Build with PlatformIO (`pio run`) to verify compilation
- No historic references in comments or documentation: do not use "initial", "previously", "no longer", "now", "before", "after", "was", "became", "replaces the old", "the current", or "Fase X" markers. Write in the timeless present tense — describe the system as it is, not how it came to be. This ban covers the code's own history. References to program state are allowed: "the privilege mode determined earlier in the execution" describes runtime state, not code history.
- No changelogs, phase markers, or technical decisions in source comments: changelogs belong in git commit messages, "Fase X" markers belong nowhere, and technical decisions belong in this file
- Every `.cpp` and `.h` file must contain the copyright header with SPDX license identifier
- Use `#pragma once` in headers instead of include guards
- Every function must have a (possibly 1-line) description of what it does
- Before writing new code, search for existing functionality to prevent code duplication
- No hard line-wrapping in markdown: All sentences of a single paragraph belong on one line.
- Run [`../tools/bump-version.sh`](tools/bump-version.sh) on a code change commit.
