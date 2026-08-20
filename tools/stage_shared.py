"""Stage the canonical shared library locally to avoid Windows Unicode path bugs.

PlatformIO invokes the Xtensa compiler with relative paths for libraries under a
project's ``lib`` directory.  This matters with older Windows toolchains that
cannot reopen an external source whose absolute path contains non-ASCII text.
The canonical sources remain exclusively in ``shared/DmsCommon``.
"""

from pathlib import Path
from shutil import copy2
import subprocess

Import("env")  # type: ignore[name-defined]  # Provided by PlatformIO/SCons.

project_dir = Path(env.subst("$PROJECT_DIR"))
repository_dir = project_dir.parent
source_dir = repository_dir / "shared" / "DmsCommon"
target_dir = project_dir / "lib" / "DmsCommon"

if not (source_dir / "library.json").is_file():
    raise RuntimeError(f"DmsCommon canonical source not found: {source_dir}")

for source in source_dir.rglob("*"):
    if not source.is_file():
        continue
    relative = source.relative_to(source_dir)
    target = target_dir / relative
    target.parent.mkdir(parents=True, exist_ok=True)
    if not target.exists() or source.read_bytes() != target.read_bytes():
        copy2(source, target)

try:
    git_sha = subprocess.check_output(
        ["git", "rev-parse", "--short=12", "HEAD"], cwd=repository_dir, text=True
    ).strip()
    if subprocess.check_output(["git", "status", "--porcelain"], cwd=repository_dir, text=True).strip():
        git_sha += "-dirty"
except (OSError, subprocess.SubprocessError):
    git_sha = "unknown"

generated_dir = project_dir / ".pio" / "generated"
generated_dir.mkdir(parents=True, exist_ok=True)
(generated_dir / "BuildInfo.h").write_text(
    '#pragma once\n#define DMS_FIRMWARE_VERSION "0.1.0"\n'
    f'#define DMS_GIT_SHA "{git_sha}"\n', encoding="ascii"
)
env.Append(CPPPATH=[str(generated_dir)])
