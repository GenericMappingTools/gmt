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
- [ ] update [changelog](https://github.com/GenericMappingTools/gmt/blob/master/doc/rst/source/changes.rst)
- [ ] check installation instructions in [INSTALL.md](https://github.com/GenericMappingTools/gmt/blob/master/INSTALL.md)
- [ ] check if there are any warnings when building the documentation
- [ ] add one new entry in `doc/rst/_static/version_switch.js` if it's a minor release
- [ ] check/set values in `cmake/ConfigDefault.cmake`
    - [ ] `GMT_VERSION_YEAR` is current year
    - [ ] `GMT_PACKAGE_VERSION_*` is correctly set
    - [ ] `GMT_LIB_SOVERSION` is correctly set
    - [ ] set `GMT_PACKAGE_VERSION_SUFFIX` to rcx
    - [ ] set `GMT_PUBLIC_RELEASE` to `TRUE`
- [ ] freeze codes and commit all changes to GitHub

**Release**:

- [ ] create source tarballs (tar.gz and tar.xz) (@PaulWessel, @meghanrjones)
- [ ] create macOS bundle (@PaulWessel, @meghanrjones)
- [ ] create Windows installers (win64) (@joa-quim)
- [ ] check if the source tarballs, macOS bundle and Windows installers work well
- [ ] upload source tarballs, macOS bundle, Windows installers to the GMT FTP (@PaulWessel, @meghanrjones)
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
- [ ] make announcements on the [GMT twitter](https://twitter.com/gmt_dev)
- [ ] update links on the main site (News, Download & Documentation)

**After release**:

- [ ] comment the `GMT_PACKAGE_VERSION_SUFFIX` line in `cmake/ConfigDefault.cmake`
- [ ] comment the `set (GMT_PUBLIC_RELEASE TRUE)` line
- [ ] commit changes to GitHub

**3rd-party update**

**Volunteers needed!** Please let us know if you volunteer to help to maintain GMT in these 3rd-party tools.

- [ ] update [conda-forge feedstock](https://github.com/conda-forge/gmt-feedstock) (@leouieda, @seisman, @weiji14)

---

- [ ] Party :tada: (don't tick before all other checkboxes are ticked!)
