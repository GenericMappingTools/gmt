---
name: GMT release checklist
about: Checklist for a new GMT release.
title: 'Release GMT x.x.x'
labels: ''
assignees: ''

---

**Version**:  x.x.x

**Before release**:

- [ ] reserve a DOI on [zenodo](https://zenodo.org) (@PaulWessel)
- [ ] run `src/gmt_make_*.sh` to update some .c and .h files
- [ ] check if all tests pass on macOS, Linux and Windows
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
- [ ] make a tag and push it to github
    ```
    git tag x.x.x
    git push --tags
    ```
- [ ] go to [GitHub Release](https://github.com/GenericMappingTools/gmt/releases) and make a release. Remember to attach the source tarballs, macOS bundle and Windows installers.
- [ ] upload source tarballs, macOS bundle, Windows installers to the GMT FTP (@PaulWessel)
- [ ] upload the tarball to zenodo (@PaulWessel)
- [ ] update README and VERSION files on the GMT FTP (@PaulWessel)
- [ ] make announcements in the [GMT forum](https://forum.generic-mapping-tools.org/)
- [ ] update links on the main site (News, Download & Documentation)

**After release**:

- [ ] create branch 6.x for bug-fixes if this is a minor release (i.e. create branch 6.1 after 6.1.0 is released)
- [ ] update `GMT_PACKAGE_VERSION_*` in `cmake/ConfigDefault.cmake`
- [ ] comment the `set (GMT_PUBLIC_RELEASE TRUE)` line
- [ ] commit changes to GitHub

**3rd-party update**

- [ ] update [conda-forge feedstock](https://github.com/conda-forge/gmt-feedstock) (@leouieda, @seisman)
- [ ] update [homebrew formula](https://github.com/Homebrew/homebrew-core/blob/master/Formula/gmt.rb) (@claudiodsf, @seisman)
- [ ] update [fink package](https://github.com/fink/fink-distributions/blob/master/10.9-libcxx/stable/main/finkinfo/sci/) (@remkos)
- [ ] update [macports ports](https://github.com/macports/macports-ports/blob/master/science/gmt5/Portfile) (@seisman)
- [ ] update [the RPM repository](https://copr.fedorainfracloud.org/coprs/genericmappingtools/gmt/) (@seisman)

---

- [ ] Party :tada: (don't tick before all other checkboxes are ticked!)
