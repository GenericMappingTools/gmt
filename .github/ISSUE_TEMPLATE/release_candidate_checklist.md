---
name: GMT release candidate checklist
about: Checklist for a new GMT release candidate.
title: Release GMT x.x.xrcx
labels: ''
assignees: ''

---

**Version**:  x.x.xrcx

**Scheduled date**: XXX XX, 20XX

**Before release**:

- [ ] check if all tests pass on macOS, Linux and Windows
- [ ] check if other GMT-derived projects work well
	- [ ] MB-System (@PaulWessel)
	- [ ] GMTSAR (@PaulWessel)
	- [ ] GMT.jl (@joa-quim)
	- [ ] PyGMT (@leouieda, @seisman, @weiji14)
	- [ ] gmtmex (@PaulWessel, @joa-quim)
- [ ] run `src/gmt_make_*.sh` to update some .c and .h files
- [ ] run `admin/gs_check.sh` to test if latest ghostscript version works
- [ ] update [changelog](/doc/rst/source/changes.rst)
- [ ] update [INSTALL.md](/INSTALL.md) (only needed for major release candidates)
- [ ] check if there are any warnings when building the documentation
- [ ] add one new entry in `doc/rst/_static/version_switch.js` if it's a minor release
- [ ] check/set values in `cmake/ConfigDefault.cmake`
    - [ ] `GMT_VERSION_YEAR` is current year
    - [ ] `GMT_PACKAGE_VERSION_*` is correctly set
    - [ ] `GMT_LIB_SOVERSION` is correctly set
    - [ ] set `GMT_PUBLIC_RELEASE` to `TRUE`
    - [ ] set `GMT_PACKAGE_VERSION_SUFFIX` to rc1
    - [ ] update `GMT_VERSION_DOI`
- [ ] freeze codes and commit all changes to GitHub

**Release**:

- [ ] create source tarballs (tar.gz and tar.xz) (@PaulWessel)
- [ ] create macOS bundle (@PaulWessel)
- [ ] create Windows installers (win64) (@joa-quim)
- [ ] check if the source tarballs, macOS bundle and Windows installers work well
- [ ] upload source tarballs, macOS bundle, Windows installers to the GMT FTP (@PaulWessel)
- [ ] update README and VERSION files on the GMT FTP (@PaulWessel)
- [ ] make a tag and push it to github (**Must be done after uploading packages to the GMT FTP**)
    ```bash
    # checkout master (for minor releases) or 6.x branch (for patch releases)
    git checkout XXXX
    # create the tag x.x.x
    git tag x.x.x
    # Push tags to GitHub
    git push --tags
    ```
- [ ] make a GitHub release.
  The GitHub Actions automatically create a draft release after pushing the tag to github.
  We need to go to the [GitHub Release](https://github.com/GenericMappingTools/gmt/releases) page, and review it manually.
  - [ ] 6 files are attached as release assets (2 source tarballs, 3 installers and 1 checksum file).
  - [ ] download the checksum file and check if the checksums are correct
  - [ ] edit the draft release, set the target to the correct tag, and publish the release
- [ ] make announcements in the [GMT forum](https://forum.generic-mapping-tools.org/)
- [ ] update links on the main site (News, Download & Documentation)

**After release**:

- [ ] create branch 6.x for bug-fixes if this is a minor release (i.e. create branch 6.1 after 6.1.0 is released)
  ```
  git checkout master
  git checkout -b 6.1
  git push --set-upstream origin 6.1
  ```
- [ ] update `GMT_PACKAGE_VERSION_*` in `cmake/ConfigDefault.cmake`
- [ ] comment the `set (GMT_PUBLIC_RELEASE TRUE)` line
- [ ] commit changes to GitHub

**3rd-party update**

**Volunteers needed!** Please let us know if you volunteer to help to maintain GMT in these 3rd-party tools.

- [ ] update [conda-forge feedstock](https://github.com/conda-forge/gmt-feedstock) (@leouieda, @seisman, @weiji14)

---

- [ ] Party :tada: (don't tick before all other checkboxes are ticked!)
