.. index:: ! gmt_update

**********
gmt_update
**********

Fetch and apply GMT binary updates from a remote manifest.

Synopsis
--------

**gmt_update** [ *manifest_url* ] [ **-**\ **-dry-run** ] [ **-**\ **-allow-dirty** ]
[ **-**\ **-root** *dir* ]

**gmt_update -**\ **-show** [ *manifest_url* ] [ **-**\ **-root** *dir* ]

**gmt_update -**\ **-make-manifest** [ **-o** *file* ] [ **-**\ **-files** *list* ]
[ **-**\ **-version** *str* ] [ **-**\ **-root** *dir* ]

**gmt_update -**\ **-help** | **-h**

Description
-----------

**gmt_update** is a small, self-contained C executable shipped alongside the
regular GMT tools in ``bin/``.  It exists so that an end user can keep an
installed GMT tree up-to-date with the most recent official binaries without
having to download a full installer or rebuild from source.

The updater is **forward-only** and **version-gated**: it refuses to install
an older release on top of a newer one, and it requires an explicit
confirmation before touching any local file.  Files locked by a running GMT
process are staged on the side and applied on the next invocation, so an
update cannot corrupt a session that is currently in use.

A single executable handles three roles, selected by the first argument:

- with no subcommand (or just a *manifest_url*), it performs an *update*;
- with ``--show``, it prints a per-file diff but does not download anything;
- with ``--make-manifest``, it generates the manifest that *other* installs
  will later consume.  This second role is intended for release engineering.

Channels
--------

Each official binary release is published as a set of *channels*, one per
combination of operating system and processor architecture.  The compile-time
channel of an individual ``gmt_update`` binary is fixed and printed at the top
of the ``--help`` output.  The six supported channels are:

==========  =====================================
Channel      Description
==========  =====================================
win_x64      Windows on Intel/AMD 64-bit
win_arm64    Windows on ARM 64-bit
linux_x64    Linux on Intel/AMD 64-bit
linux_arm64  Linux on ARM 64-bit (e.g. Raspberry Pi 5, Ampere)
macos_x64    macOS on Intel
macos_arm64  macOS on Apple Silicon (M1/M2/...)
==========  =====================================

Each channel sees only its own manifest.  A ``win_x64`` ``gmt_update`` will
never download a Linux binary and vice versa.

How the manifest is located
---------------------------

The default *manifest_url* is the GitHub REST API endpoint for the GMT
project's latest release:

    https://api.github.com/repos/GenericMappingTools/gmt/releases/latest

When the updater is given an ``api.github.com/.../releases/latest`` URL it
fetches the JSON, walks the ``assets[]`` array, and follows the
``browser_download_url`` of the asset whose name is
``manifest_<channel>.txt``.  Every other file listed in that manifest is then
fetched as a *sibling* of the manifest itself, so a release simply consists of
the manifest plus all referenced binaries uploaded to the same location.

A literal URL pointing directly at a ``manifest_*.txt`` is also accepted and
used verbatim; this is what you want when testing against a private mirror.

.. note::
   The updater never constructs a static release-asset URL on its own.  This
   is a deliberate design choice: GitHub does not provide a stable
   ``latest/<asset>`` URL, and any attempt to invent one will produce
   ``404`` errors as soon as the release name changes.  Always go through
   the API.

Options
-------

Common options
~~~~~~~~~~~~~~

*manifest_url*
    URL of the manifest to read.  Defaults to the GitHub API endpoint
    described above.  May also be the direct URL of a published
    ``manifest_<channel>.txt`` (useful for staging tests).

**-**\ **-root** *dir*
    Override the install root.  By default the updater resolves its own
    executable path (following symlinks), strips the trailing ``bin/``
    component, and uses the resulting directory as the install root.  All
    relative paths in the manifest are interpreted under this root.

**-**\ **-help**, **-h**
    Print a compact synopsis to standard output and exit.

Update mode
~~~~~~~~~~~

**-**\ **-dry-run**
    Resolve the manifest, fetch and parse it, and print the per-file diff,
    but do not download or replace anything.  No confirmation prompt is
    shown.  This is the safe way to preview an upgrade.

**-**\ **-allow-dirty**
    Permit running against a developer build whose version string contains
    ``-dirty`` (i.e. built from an uncommitted working tree).  Without this
    flag such builds are refused, because the version comparison against the
    manifest cannot be trusted.

Manifest-generation mode (``-`` ``-make-manifest``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**-o** *file*
    Write the generated manifest to *file*.  When omitted, the manifest is
    written to ``./manifest_<channel>.txt`` in the current directory — the
    same basename the consumer side resolves via the GitHub API, so the file
    can be uploaded to the release as-is.

**-**\ **-files** *list*
    A plain-text file with one repository-relative path per line; lines
    starting with ``#`` and blank lines are ignored.  Overrides the
    built-in list of artefacts for the current OS.

**-**\ **-version** *str*
    Use *str* as the manifest ``version:`` field instead of running
    ``<root>/bin/gmt --version`` to probe it.

**-**\ **-root** *dir*
    Hash files under *dir* instead of under the updater's own install root.
    Useful for packaging a build that lives in a staging directory.

Version gate
------------

Before any download decision, the updater spawns
``<install_root>/bin/gmt --version`` and parses the output to obtain the
local version (for example, ``6.7.0_d3435c5_2026.05.19``, or
``6.7.0_8b76b26-dirty_...`` on a developer build).  It then compares that to
the manifest's ``version:`` field using a *(semver, date)* tuple:

- if the local version is newer than the manifest, the updater refuses to
  proceed — there is nothing to upgrade to;
- if the local version is equal, only individual files whose SHA-256 differs
  from the manifest will be replaced;
- if the manifest is newer, the updater offers to upgrade.

Dirty builds (``-dirty`` in the version string) are rejected unless
``--allow-dirty`` is passed.

Confirmation prompt
-------------------

After the diff is computed the updater prints a summary of the form ::

    N file(s) will be downloaded and replace local copies under
      <install_root>
    Proceed? [y/N]

and waits on standard input.  Anything other than ``y`` (or ``Y``) aborts
cleanly without touching the filesystem.

If any of the stale files turn out to be *newer on disk* than the manifest's
build timestamp (typically because the user just rebuilt that DLL locally),
a second prompt appears ::

    WARNING: K of those files are NEWER on disk than the manifest's
             build timestamp (<ISO-8601>).
             Replacing them would OVERWRITE local newer copies
             (typically a just-rebuilt developer DLL).
    Also overwrite the K locally-newer file(s) (DOWNGRADE)? [y/N]

A default-``no`` answer keeps the local copies and tags them ``KEEP-LOCAL``
in the apply phase; the non-regression files still update.

Integrity checks
----------------

Each downloaded file is written to a temporary ``<name>.new`` sidecar and its
SHA-256 is recomputed before the swap.  If the hash does not match the
manifest, the temporary file is discarded and the original is left untouched.
The manifest is the trust anchor and must be served over HTTPS; the
SHA-256 column binds content to identity.

Locked files
------------

If the destination file is held open by another process (most commonly the
``gmt.exe`` you are about to upgrade, or a long-running script that has
loaded ``libgmt``), the swap is deferred:

- on Windows, ``MoveFileExA`` with ``MOVEFILE_REPLACE_EXISTING |
  MOVEFILE_DELAY_UNTIL_REBOOT`` is attempted, and the validated download is
  also staged next to the target as ``<file>.pending``.  The next run of
  ``gmt_update`` will apply that pending file as soon as the lock is gone;
- on Linux/macOS, the staged ``<file>.pending`` is renamed into place on the
  next run.

On Windows the updater additionally calls the Restart Manager API to report
which process IDs currently hold each locked file.

Manifest format
---------------

The manifest is a plain-text file, one record per line; blank lines and
lines starting with ``#`` are ignored.  Header records appear once::

    version: <gmt-version-string>
    build:   <ISO-8601 UTC timestamp>
    channel: <os>_<arch>

Followed by one file record per updatable artefact::

    file <relpath-from-install-root> <size-bytes> <sha256-hex-64chars>

An optional fifth field on a ``file`` line is interpreted as a
per-file download URL override, useful when a single file must be served
from a different host.  When omitted (the normal case), each file is fetched
as a sibling of the manifest URL itself.

The manifest is terminated by ::

    end

Examples
--------

**1. Check what an update would do (no changes made):** ::

    gmt_update --dry-run

**2. Apply the latest official release:** ::

    gmt_update

When run from a default Windows install, this resolves the GitHub API,
locates the ``manifest_win_x64.txt`` asset of the latest release, diffs each
listed binary against your installed ``bin/gmt.exe``, ``bin/gmt_w64.dll``
and friends, prompts ``Proceed? [y/N]``, and on ``y`` downloads and swaps in
just the binaries that have actually changed.

**3. Pin to a specific manifest URL (mirror / staging):** ::

    gmt_update https://my-mirror.example.com/gmt/6.7.0/manifest_win_x64.txt

**4. Inspect the diff verbosely without ever connecting to do an update:** ::

    gmt_update --show

This prints a table such as ::

    current  bin/gmt.exe                              0a1b2c3d4e5f 0a1b2c3d4e5f
    STALE    bin/gmt_w64.dll                          deadbeefcafe feedfacecafe yes
    summary: 1 stale, 0 missing, 1 pending

**5. Update an installation that is not where the updater lives:** ::

    gmt_update --root D:\opt\gmt-staging

**6. Generate the manifest for a release you just built locally:** ::

    cd <release-staging>
    gmt_update --make-manifest

The above writes ``./manifest_<channel>.txt`` in the current directory.
Upload that file together with the referenced binaries to the same release
(or the same directory on a private mirror) and the consumer side is done.

**7. Generate a manifest from a custom file list and a hand-set version:** ::

    gmt_update --make-manifest -o manifest_win_x64.txt --files my_files.txt --version 6.7.0_rc1

with ``my_files.txt`` containing, e.g. ::

    # Core binaries
    bin/gmt.exe
    bin/gmt_w64.dll
    bin/postscriptlight_w64.dll
    bin/gmt_plugins/supplements_w64.dll


Security notes
--------------

- The manifest is the only trust anchor.  Always serve it over HTTPS.  The
  SHA-256 column binds each binary's identity to its content; an attacker
  who can substitute a binary but not the manifest cannot make the updater
  swap it in.
- The updater never executes any downloaded file; it only renames it into
  place.
- The GitHub REST endpoint is reached anonymously; no token is required.

See Also
--------

:doc:`gmt`,
`GMT releases on GitHub <https://github.com/GenericMappingTools/gmt/releases>`_
