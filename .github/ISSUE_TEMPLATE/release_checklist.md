---
name: GMT Release Checklist
about: Checklist for new GMT release.
title: ''
labels: ''
assignees: ''

---

**Version**:  x.x.x

**Before release**:

- [ ] run [coverity scan](https://scan.coverity.com/projects/gmt) and fix defects (@PaulWessel)
- [ ] update changelog
- [ ] check/change `cmake/ConfigDefault.cmake`

    - [ ] `GMT_VERSION_YEAR` is current year
    - [ ] `GMT_PACKAGE_VERSION_*` is correctly set
    - [ ] `GMT_LIB_SOVERSION` is correctly set
    - [ ] set `GMT_PUBLIC_RELEASE` to `TRUE`

- [ ] freeze codes and commit all changes to GitHub

**Release**:

- [ ] create source packages (tar.gz and tar.xz) (@PaulWessel)
- [ ] create macOS Bundle (@PaulWessel)
- [ ] create Windows binary packages (win32 and win64) (@joa-quim)
- [ ] make a tag and push it to github

    ```
    git tag 6.0.0
    git push --tag
    ```

- [ ] go to [GitHub Release](https://github.com/GenericMappingTools/gmt/releases) and make a release.
      Remember to attach the soruce packages, Windows binary packages and macOS Bundle.
- [ ] upload source packages, Windows binary packages and macOS Bundle to the GMT FTP (@PaulWessel)
- [ ] update README and VERSION files on the GMT FTP

- [ ] update conda packages via [conda-forge/gmt-feedstock](https://github.com/conda-forge/gmt-feedstock) (@leouieda, @seisman)
- [ ] update [homebrew formula](https://github.com/Homebrew/homebrew-core/blob/master/Formula/gmt.rb) (@seisman)
- [ ] update fink package (@remkos)
- [ ] update macports ports
- [ ] announcements

**After release**:

- [ ] update `GMT_PACKAGE_VERSION_*` in `cmake/ConfigDefault.cmake`
- [ ] set `GMT_PUBLIC_RELEASE` to `FALSE`

---

- [ ] Party :tada: (don't tick before all other checkboxes are ticked!)
