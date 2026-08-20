"""Stage the canonical shared library locally to avoid Windows Unicode path bugs.

PlatformIO invokes the Xtensa compiler with relative paths for libraries under a
project's ``lib`` directory.  This matters with older Windows toolchains that
cannot reopen an external source whose absolute path contains non-ASCII text.
The canonical sources remain exclusively in ``shared/DmsCommon``.
"""

from pathlib import Path
from shutil import copy2

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
