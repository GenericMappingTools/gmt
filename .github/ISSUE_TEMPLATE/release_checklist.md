---
name: GMT Release Checklist
about: Checklist for new GMT release.
title: ''
labels: ''
assignees: ''

---

**Version**:  x.x.x

Things to do before release:

- [ ] check `GMT_VERSION_YEAR` in `cmake/ConfigDefault.cmake`
- [ ] check `GMT_PACKAGE_VERSION_*` in `cmake/ConfigDefault.cmake`
- [ ] check `GMT_LIB_SOVERSION` in `cmake/ConfigDefault.cmake`
- [ ] set `GMT_SOURCE_CODE_CONTROL_VERSION_STRING` in `cmake/ConfigDefault.cmake`
- [ ] create source packages (tar.gz and tar.xz) (@PaulWessel)
- [ ] create macOS Bundle (@PaulWessel)
- [ ] create Windows binary packages (win32 and win64) (@joa-quim)
- [ ] upload source packages, Windows binary packages and macOS bundle to FTP (@PaulWessel)
- [ ] update conda packages via [conda-forge/gmt-feedstock](https://github.com/conda-forge/gmt-feedstock) (@leouieda, @seisman)
- [ ] update [homebrew formula](https://github.com/Homebrew/homebrew-core/blob/master/Formula/gmt.rb) (@seisman)
- [ ] update fink package
- [ ] announcements

Things to do after release:

- [ ] update `GMT_PACKAGE_VERSION_*` in `cmake/ConfigDefault.cmake`
- [ ] uncomment `GMT_SOURCE_CODE_CONTROL_VERSION_STRING` in `cmake/ConfigDefault.cmake`

---

- [ ] Party :tada: (don't tick before all other checkboxes are ticked!)
