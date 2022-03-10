Maintainers Guide
=================

This page contains instructions for project maintainers about making releases, creating source packages, managing CI, etc.

If you want to make a contribution to the project, see the :doc:`Contributing Guide </devdocs/contributing>` instead.

Creating source packages
------------------------

Edit ``cmake/ConfigDefault.cmake`` and set ``GMT_PACKAGE_VERSION_MAJOR``, ``GMT_PACKAGE_VERSION_MINOR``, and
``GMT_PACKAGE_VERSION_PATCH``. Also set ``GMT_PUBLIC_RELEASE`` to ``TRUE``. Then create source packages with::

   cmake --build . --target gmt_release      # export the source tree and documentation
   cmake --build . --target gmt_release_tar  # create tarballs (in tar.gz and tar.xz formats)

Packaging
---------

Currently, packaging with CPack works on macOS (Bundle, TGZ, TBZ2), Windows (ZIP, NSIS), and UNIX (TGZ, TBZ2). On
Windows you need to install `NSIS <http://nsis.sourceforge.io/>`_. After building GMT and the documentation,
build and place the executables, including the supplements, with::

   cmake --build . --target install


and then create the package with either one of these:: 

   cmake --build . --target package
   cpack -G TGZ|TBZ2|Bundle|ZIP|NSIS

Continuous Integration
----------------------

We use GitHub Actions Continuous Integration (CI) services to build and test the project on Linux, macOS, and Windows.

There are 11 configuration files located in ``.github/workflows/``:

1. ``backport.yml`` (Backports Pull Requests with specific labels into the matching branch)

   This workflow backports Pull Requests labelled by "backport xxx" into the "xxx" branch. For example, the workflow
   backports Pull Requests labelled "backport 6.1" into the "6.1" branch. This workflow does not apply to Pull Requests
   from forks (`issue #3827 <https://github.com/GenericMappingTools/gmt/issues/3827>`_).

2. ``build.yml`` (Build GMT and run a few simple tests)

   This workflow is run on every commit to the *master* branch and Pull Request branches. The workflow configures
   and builds GMT on Linux, macOS, and Windows and runs some simple tests. The workflow uses the scripts in the ``ci/``
   directory for downloading/installing dependencies (``ci/download-coastlines.sh`` and ``ci/install-dependencies-*.sh``),
   configuring GMT (``ci/config-gmt-*.sh``), building GMT, and running some simple tests (``ci/simple-gmt-tests.*``).

3. ``check-links.yml`` (Check links in the repository and website)

   This workflow is run every Sunday at 12:00 (UTC) to check all external links in plaintext and HTML files. It will
   create an issue if broken links are found.

4. ``ci-caches.yml`` (Cache GMT remote data files and Windows vcpkg libraries needed for GitHub Actions CI)

   This workflow is run every Sunday at 12:00 (UTC). If new remote files are needed urgently, maintainers can
   manually uncomment the 'pull_request:' line in the ``ci-caches.yml`` file to refresh the cache. The workflow uses the
   script ``ci/install-dependencies-windows.sh`` to cache the vcpkg libraries.

5. ``code-validator.yml`` (Validate code consistency)

   This workflow is run on every commit to the *master* branch and Pull Request branches. It runs the scripts
   ``src/gmt_make_PSL_strings.sh``, ``src/gmt_make_enum_dicts.sh``, and ``src/gmt_make_module_purpose.sh`` and exits
   with an error if any changes are detected by git. It also checks the GMT version year in ``ConfigDefault.cmake`` and
   the permissions of the bash scripts.

6. ``docker.yml`` (Build GMT on different Linux distros using dockers)

   This workflow is run when Pull Requests are merged into the *master* branch. It downloads/installs dependencies
   and configures and builds GMT on different Linux distributions using dockers.

7. ``docs.yml``  (Build documentation on Linux/macOS/Windows)

   This workflow is run when Pull Requests are merged into the *master* branch, if the Pull Request involved changes to
   files in the ``doc/``, ``.github/workflows/``, or ``ci/`` directories.

   The workflow also handles the documentation deployment:

   * Updating the development documentation by pushing the built HTML pages from the *master* branch into the ``dev``
     folder of the *gh-pages* branch.
   * Updating the ``latest`` documentation link to the new release.

8. ``draft-release.yml`` (Drafts the next release notes)

   This workflow is run to draft the next release notes when new tags are made. It downloads GMT tarballs, macOS bundle
   and Windows installers from the GMT FTP server, calculate their sha256sums and save into `gmt-X.Y.Z-checksums.txt`,
   draft the release notes, and uploads them as release assets. Maintainers still need to review the draft release
   notes and click the "publish" button.

9. ``lint-checker.yml`` (Run cppchecks)

   This workflow is run every day at 12:00 (UTC) to run `admin/run_cppcheck.sh` on the source code.

10. ``scm-check.yml`` (Check for new scientific color maps releases)

    This workflows is run every Sunday at 12:00 (UTC) to check whether there has been a new release of the
    `Scientific colour maps <http://www.fabiocrameri.ch/colourmaps.php>`_. If a new release is found, it will open an
    issue automatically.

11. ``tests.yml`` (Tests on Linux/macOS/Windows)

    This workflow is run when Pull Requests are merged into the *master* branch, if the Pull Request involved changes
    to the folders that contain source code, workflows, tests, or scripts for generating documentation figures. It runs
    the full GMT test suite on Linux, macOS, and Windows.

12. ``release-drafter.yml`` (Drafts the next release notes)

    This workflow is run to update the next releases notes as pull requests are merged into master.

Updating the changelog
----------------------

The Release Drafter GitHub Action will automatically keep a draft changelog at
https://github.com/GenericMappingTools/gmt/releases, adding a new entry every time a Pull Request (with a proper label)
is merged into the master branch. This release drafter tool has two configuration files, one for the GitHub Action
at ``.github/workflows/release-drafter.yml``, and one for the changelog template at ``.github/release-drafter.yml``.
Configuration settings can be found at https://github.com/release-drafter/release-drafter. The maintenance documentation
for this workflow is based on the `PyGMT Maintenance Documentation <https://www.pygmt.org/dev/maintenance.html>`_.

The drafted release notes are not perfect, so we will need to tidy it prior to publishing the actual release notes at
https://docs.generic-mapping-tools.org/latest/changes.html.

1. Go to https://github.com/GenericMappingTools/gmt/releases and click on the 'Edit' button next to the current draft
   release note. Copy the text of the automatically drafted release notes under the 'Write' tab to
   ``doc/rst/source/changes.rst``.
2. Open a new Pull Request using the title 'Changelog entry for GMT X.Y.Z' with the updated release notes, so that other
   people can help to review and collaborate on the changelog curation process described next.
3. Edit the change list to remove any trivial changes (updates to the README, typo fixes, CI configuration, etc).
4. Edit the formatting to use :doc:`/devdocs/rst-cheatsheet`.
5. Add links in the changelog to elements of the documentation as appropriate.
