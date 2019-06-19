---
name: GMT Release Checklist
about: Checklist for new GMT release.
title: 'Release GMT x.x.x'
labels: ''
assignees: ''

---

**Version**:  x.x.x

**Before release**:

- [ ] run `src/gmt_make_*.sh` to update some .c and .h files
- [ ] check if all tests pass on macOS, Linux and Windows
- [ ] update changelog
- [ ] update INSTALL.md
- [ ] build documentations and fix warnings if any
- [ ] check/set values in `cmake/ConfigDefault.cmake`
    - [ ] `GMT_VERSION_YEAR` is current year
    - [ ] `GMT_PACKAGE_VERSION_*` is correctly set
    - [ ] `GMT_LIB_SOVERSION` is correctly set
    - [ ] set `GMT_PUBLIC_RELEASE` to `TRUE`
- [ ] freeze codes and commit all changes to GitHub

**Release**:

- [ ] create source tarballs (tar.gz and tar.xz) (@PaulWessel)
- [ ] create macOS bundle (@PaulWessel)
- [ ] create Windows installers (win32 and win64) (@joa-quim)
- [ ] make a tag and push it to github
    ```
    git tag x.x.x
    git push --tags
    ```
- [ ] go to [GitHub Release](https://github.com/GenericMappingTools/gmt/releases) and make a release. Remember to attach the source tarballs, macOS bundle and Windows installers.
- [ ] upload source tarballs, macOS bundle, Windows installers to the GMT FTP (@PaulWessel)
- [ ] update README and VERSION files on the GMT FTP
- [ ] announcements

**After release**:

- [ ] update `GMT_PACKAGE_VERSION_*` in `cmake/ConfigDefault.cmake`
- [ ] set `GMT_PUBLIC_RELEASE` to `FALSE`

**3rd-party update**

- [ ] update conda packages via [conda-forge/gmt-feedstock](https://github.com/conda-forge/gmt-feedstock) (@leouieda, @seisman)
- [ ] update [homebrew formula](https://github.com/Homebrew/homebrew-core/blob/master/Formula/gmt.rb) (@seisman)
- [ ] update fink package (@remkos)
- [ ] update macports ports

---

- [ ] Party :tada: (don't tick before all other checkboxes are ticked!)
