# Target Review Report (File or Folder)

You are reviewing a selected **target** in this codebase: either a **file** or a **folder**.

## Inputs
- Target: **$ARGUMENTS**
  - If `$ARGUMENTS` points to a file, review that file.
  - If `$ARGUMENTS` points to a folder, review all files in that folder (recursively unless that would be clearly wrong for this repo structure).
  - If `$ARGUMENTS` is empty, use the current editor/selection context:
    - If a file is open/selected: review that file.
    - If a folder is selected in the explorer: review that folder.

## Task
Review the target and produce **only** a review report.

Focus on:
- Issues/bugs and correctness risks
- Cleanups (redundant/dead code, unused includes, stale comments)
- Legacy or migration leftovers (compat shims, transitional macros, commented-out blocks, TODOs tied to old migrations)
- Efficiency/performance concerns (allocations, copies, repeated computations, hot loops)
- Small, safe refactors that improve clarity/coupling without changing behavior

## Constraints
- Do **not** implement changes. Do **not** provide a step-by-step plan. Output must be a report only.
- Do **not** change behavior in suggestions unless you clearly label it as behavior-changing.
- Prefer removing code only if you can show it’s unused (or label uncertainty + how to confirm).
- Keep embedded constraints in mind (RAM/flash, runtime cost).

## Output format (return ONLY this report)
### A. Summary
- 5–10 bullets, highest impact first

### B. Scope
- State whether the target was a **file** or **folder**
- If folder: whether review was recursive and any exclusions

### C. Inventory
- If folder: list files (group by module/type)
- 1–2 lines per file: purpose + key dependencies within the target (and any notable external ones)
- If file: summarize key symbols (classes/functions) and dependencies

### D. Findings (by file)
For each finding, use tags: **[bug] [cleanup] [perf] [style] [refactor]**
- File: `path/filename`
  - Tag: description
  - Evidence: point to the relevant symbol/function/line area (describe, don’t paste whole files)
  - Suggested fix: brief, minimal

### E. Proposed changes (checklist, ordered safe→risky)
- [ ] Item…

### F. Quick wins (<30 min)
- Bullet list

### G. Risky changes (needs careful testing)
- Bullet list + why risky + what to test

### H. Open questions / uncertainties
- What you couldn’t prove, and how I can confirm (grep targets, call sites, build flags)

## Start
Resolve the target using `$ARGUMENTS` or editor/selection context, then scan it and produce the report.