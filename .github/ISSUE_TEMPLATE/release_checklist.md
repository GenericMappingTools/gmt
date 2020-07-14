---
name: GMT release checklist
about: Checklist for a new GMT release.
title: 'Release GMT x.x.x'
labels: ''
assignees: ''

---

**Version**:  x.x.x

**Scheduled date**: XXX XX, 20XX

**Before release**:

- [ ] check if all tests pass on macOS, Linux and Windows
- [ ] check if other GMT-derived projects work well
	- [ ] MB-System (@PaulWessel)
	- [ ] GMTSAR (@PaulWessel)
	- [ ] GMT.jl (@joa-quim)
	- [ ] PyGMT (@leouieda, @seisman, @weiji14)
	- [ ] gmtmex (@PaulWessel, @joa-quim)
- [ ] reserve a DOI on [zenodo](https://zenodo.org) (@PaulWessel)
- [ ] run `src/gmt_make_*.sh` to update some .c and .h files
- [ ] run `admin/gs_check.sh` to test if latest ghostscript version works
- [ ] update changelog
- [ ] update INSTALL.md
- [ ] check if there are any warnings when build the documentation
- [ ] check/set values in `cmake/ConfigDefault.cmake`
    - [ ] `GMT_VERSION_YEAR` is current year
    - [ ] `GMT_PACKAGE_VERSION_*` is correctly set
    - [ ] `GMT_LIB_SOVERSION` is correctly set
    - [ ] set `GMT_PUBLIC_RELEASE` to `TRUE`
    - [ ] update `GMT_VERSION_DOI`
- [ ] freeze codes and commit all changes to GitHub

**Release**:

- [ ] create source tarballs (tar.gz and tar.xz) (@PaulWessel)
- [ ] create macOS bundle (@PaulWessel)
- [ ] create Windows installers (win32 and win64) (@joa-quim)
- [ ] check if the source tarballs, macOS bundle and Windows installers work well
- [ ] upload source tarballs, macOS bundle, Windows installers to the GMT FTP (@PaulWessel)
- [ ] update README and VERSION files on the GMT FTP (@PaulWessel)
- [ ] make a tag and push it to github (**Must be done after uploading packages to the GMT FTP**)
    ```
    git tag x.x.x
    git push --tags
    ```
- [ ] make a GitHub release.
  Go to the [GitHub Release](https://github.com/GenericMappingTools/gmt/releases) page,
  check the draft release (automatically created by github-actions after pushing the tag).
  - [ ] 6 files are attached as release assets (2 source tarballs, 3 installers and 1 checksum file).
  - [ ] download the checksum file and check if the checksums are correct
  - [ ] edit the draft release, set the target to the correct tag, and publish the release
- [ ] upload the tarball to zenodo (@PaulWessel)
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

- [ ] update [conda-forge feedstock](https://github.com/conda-forge/gmt-feedstock) (@leouieda, @seisman)
- [ ] update [homebrew formula](https://github.com/Homebrew/homebrew-core/blob/master/Formula/gmt.rb) (@claudiodsf, @seisman)
- [ ] update [macports ports](https://github.com/macports/macports-ports/blob/master/science/gmt5/Portfile) (@remkos, @seisman)
- [ ] update [the RPM repository](https://copr.fedorainfracloud.org/coprs/genericmappingtools/gmt/) (@seisman)
- [ ] update the [try-gmt](https://github.com/GenericMappingTools/try-gmt) Jupyter lab (@seisman)
- [ ] update [the AUR repository](https://aur.archlinux.org/packages/gmt6/) (@holishing)

---

- [ ] Party :tada: (don't tick before all other checkboxes are ticked!)
