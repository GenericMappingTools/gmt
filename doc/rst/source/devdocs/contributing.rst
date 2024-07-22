:orphan:

Contributors Guide
==================

This is a community driven project and everyone is welcome to contribute. The project is hosted at the
`GMT GitHub repository <https://github.com/GenericMappingTools/gmt>`_.

The goal is to maintain a diverse community that's pleasant for everyone. **Please be considerate and respectful of
others**. Everyone must abide by our `Code of Conduct <https://github.com/GenericMappingTools/.github/blob/main/CODE_OF_CONDUCT.md>`_
and we encourage all to read it carefully.

Ways to Contribute
------------------

Ways to Contribute Documentation and/or Code
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Tackle any `issue <https://github.com/GenericMappingTools/gmt/issues>`_ that you wish! Please leave a comment on the
  issue indicating that you want to work on it. Some issues are labeled as
  `"good first issues" <https://github.com/GenericMappingTools/gmt/issues?q=is%3Aopen+is%3Aissue+label%3A%22good+first+issue%22>`_
  to indicate that they are beginner friendly, meaning that they don't require extensive knowledge of the project.
- Make a tutorial or example of how to do something.
- Improve the API documentation.
- Contribute code you already have. It doesn't need to be perfect! We will help you clean things up, test it, etc.

Ways to Contribute Feedback
~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Provide feedback about how we can improve the project or about your particular use case. Open an
  `issue <https://github.com/GenericMappingTools/gmt/issues>`_ with feature requests of bug fixes, or post general
  comments/questions on the  `forum <https://forum.generic-mapping-tools.org/>`_.
- Help triage issues, or give a "thumbs up" on issues that others reported which are relevant to you.

Ways to Contribute to Community Building
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Participate and answer questions on the `GMT Community Forum <https://forum.generic-mapping-tools.org/>`_.
- Participate in discussions at the monthly GMT Community Meetings, which are announced on the
  `forum governance page <https://forum.generic-mapping-tools.org/c/governance/>`_.
- Cite GMT when using the project.
- Spread the word about GMT or star the project!

Providing Feedback
------------------

Reporting a Bug
~~~~~~~~~~~~~~~

- Find the `Issues <https://github.com/GenericMappingTools/gmt/issues>`_ tab on the top of the GitHub repository and
  click *New Issue*.
- Click on *Get started* next to *Bug report*.
- **Please try to fill out the template with as much detail as you can**.
- After submitting your bug report, try to answer any follow up questions about the bug as best as you can.

Submitting a Feature Request
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* Find the `Issues <https://github.com/GenericMappingTools/gmt/issues>`_ tab on the top of the GitHub repository and
  click *New Issue*.
* Click on *Get started* next to *Feature request*.
* **Please try to fill out the template with as much detail as you can**.
* After submitting your feature request, try to answer any follow up questions as best as you can.

Submitting General Comments/Questions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There are several pages on the `Community Forum <https://forum.generic-mapping-tools.org/>`_
where you can submit general comments and/or questions:

* For questions about using GMT, select *New Topic* from the
  `Q&A Page <https://forum.generic-mapping-tools.org/c/questions/>`_.
* For general comments, select *New Topic* from the
  `Lounge Page <https://forum.generic-mapping-tools.org/c/lounge/>`_.
* To share your work, select *New Topic* from the
  `Showcase Page <https://forum.generic-mapping-tools.org/c/Show-your-nice-example-script/>`_.

General Guidelines
------------------

Resources for New Contributors
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Please take a look at these resources to learn about Git and pull requests (don't
hesitate to :ref:`ask for help <devdocs/contributing:Getting Help>`):

- `How to Contribute to Open Source <https://opensource.guide/how-to-contribute/>`_.
- `Git Workflow Tutorial <http://www.asmeurer.com/git-workflow/>`_ by Aaron Meurer.
- `How to Contribute to an Open Source Project on GitHub <https://egghead.io/courses/how-to-contribute-to-an-open-source-project-on-github>`_.

Getting Help
~~~~~~~~~~~~

Discussion often happens in the issues and pull requests. For general questions, you can post on the
`GMT Community Forum <https://forum.generic-mapping-tools.org/>`_. We also host community meetings roughly monthly
to discuss GMT development, which are announced on the `GMT Community Forum <https://forum.generic-mapping-tools.org/>`_.

Pull Request Workflow
~~~~~~~~~~~~~~~~~~~~~

We follow the `git pull request workflow <http://www.asmeurer.com/git-workflow/>`_ to make changes to our codebase.
Every change made goes through a pull request, even our own, so that our
`continuous integration <https://en.wikipedia.org/wiki/Continuous_integration>`_ services have a change to check that
the code is up to standards and passes all our tests. This way, the *master* branch is always stable.

General Guidelines for Making a Pull Request (PR):
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

- **Open an issue first** describing what you want to do. If there is already an issue that matches your PR, leave a
  comment there instead to let us know what you plan to do.
- Each pull request should consist of a **small** and logical collection of changes.
- Larger changes should be broken down into smaller components and integrated separately.
- Bug fixes should be submitted in separate PRs.
- Describe what your PR changes and *why* this is a good thing. Be as specific as you can. The PR description is how we
  keep track of the changes made to the project over time.
- Do not commit changes to files that are irrelevant to your feature or bugfix (eg: `.gitignore`, IDE project files, etc).
- Write descriptive commit messages. Chris Beams has written a `guide <https://chris.beams.io/posts/git-commit/>`_ on
  how to write good commit messages.
- Be willing to accept criticism and work on improving your code; we don't want to break other users' code, so care
  must be taken to not introduce bugs.
- Be aware that the pull request review process is not immediate, and is generally proportional to the size of the pull
  request.

General Process for Pull Request Review:
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

After you've submitted a pull request, you should expect to hear at least a comment within a couple of days.
We may suggest some changes or improvements or alternatives.

Some things that will increase the chance that your pull request is accepted quickly:

- Write a good and detailed description of what the PR does.
- Write tests for the code you wrote/modified.
- Readable code is better than clever code (even with comments).
- Write documentation for your code and leave comments explaining the *reason* behind non-obvious things.
- Include an example of new features in the gallery or tutorials.

Pull requests will automatically have tests run by GitHub Actions. Github will show the status of these checks on the
pull request. Try to get them all passing (green). If you have any trouble, leave a comment in the PR or
:ref:`get in touch <devdocs/contributing:Getting Help>`.

Setting up your environment
---------------------------

For editing the documentation locally and contributing code, you will need a development environment. To set up a
development environment, follow the steps for `building GMT <https://github.com/GenericMappingTools/gmt/blob/master/BUILDING.md>`__
paying attention to several "Note for developers" that provide helpful or necessary information for running tests and
building the documentation.

To enable testing, you need to *uncomment* the following lines in your ``ConfigUserAdvanced.cmake`` when
`configuring GMT <https://github.com/GenericMappingTools/gmt/blob/master/BUILDING.md#configuring>`_::

  enable_testing()
  set (DO_EXAMPLES TRUE)
  set (DO_TESTS TRUE)

  set (SUPPORT_EXEC_IN_BINARY_DIR TRUE)

  set (DO_API_TESTS ON)

Optionally, uncomment the following line to run tests on the supplement modules::

  set (DO_SUPPLEMENT_TESTS ON)

Optionally, uncomment the following line and change ``4`` to the number of ctest jobs to run simultaneously::

  set (N_TEST_JOBS 4)

Updating the development source codes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Assuming you did not delete the build directory, this is just as simple as::

  cd <path-to-gmt>
  git pull
  cd build
  cmake --build .
  cmake --build . --target install

CMake will detect any changes to the source files and will automatically reconfigure. If you deleted all files inside
the build directory you have to run CMake again manually.

Using build and test aliases
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The file ``/admin/bashrc_for_gmt`` contains useful aliases for building and testing GMT that some developers chose to
use. New pull requests with other aliases that you find helpful are welcome. This file is version controlled, so you
should copy the file to a different location in order to edit and use it. For example, use these commands to copy it to
your home directory::

  cd <path-to-gmt>
  cp admin/bashrc_for_gmt ~/.bashrc_for_gmt

Here are the steps for setting up ``bashrc_for_gmt`` after copying it to a new location:

- If you do not have `ninja <https://ninja-build.org/>`_ installed, you will need to change ``builder=ninja`` to
  ``builder=make`` and ``Bname="Ninja"`` to ``Bname="Unix Makefiles"``. Ninja is recommended for speeding up build times.
- You may need to update ``pngview=open`` and ``pdfview=open`` depending on your preferred program for viewing files.
- Optionally, change ``ncores=4`` to the number of cores to use for building and running tests.
- Change ``MATLAB=/Applications/MATLAB_R2019a.app`` to the path for your version of the MATLAB app.
- Set ``REPO_DIR`` to the path that contains the local ``git clone`` copy of the GMT repository.
- Set ``DATA_DIR`` to the path that contains the folders ``dcw-gmt-2.0.0/`` and ``gshhg-gmt-2.3.7/`` for the dcw and gshhg
  datasets respectively. If these folders are not located in the same path, you can instead delete the line
  (``DATA_DIR=<path to directory containing GSHHG and DCW>``) and set the individual paths to the GSHHG and DCW source
  by changing (``export GMT_GSHHG_SOURCE=${DATA_DIR}/gshhg-gmt-2.3.7``) and (``export GMT_DCW_SOURCE=${DATA_DIR}/dcw-gmt-2.0.0``).
- Edit the file ``~/.bashrc`` to include the line ``source <path>/bashrc_for_gmt``. If you set up ``bashrc_for_gmt`` as a
  hidden file in your home directory, this line should be ``source ~/.bashrc_for_gmt``.

Here are some of the shortcuts included in ``bashrc_for_gmt``:

- ``gmt6`` and ``gtop`` can be used to quickly ``cd`` to the top of the GMT source directory and repository base respectively.
- ``gmtfind`` can be used to list all source, docs, scripts, and text files where a string appears in the file
  (e.g., ``gmtfind "Grid increment is"`` returns all files that contain the string 'Grid increment is'). This includes all
  files recursively from the current working directory; ``gtop`` or ``gmt6`` can be used prior to this command to get
  to the source directory or repository base.
- ``cmakegmtd``, ``cmakegmtr``, and ``cmakegmtx`` configures cmake for debug, release, and XCode debug respectively.
- ``dlog`` and ``rlog`` can be used to open the debug and release build check error logs respectively.
- There are several aliases with various combinations of pulling new changes, deleting the build directories,
  configuring cmake, and building the source code. Each of these are documented with comments in ``bashrc_for_gmt``.
- ``checkdbuild`` and ``checkrbuild`` can be used to run the tests for the debug and release builds respectively.
- ``vpngdbuild`` and ``vpdfdbuild`` can be used to open the results from all failing image-based tests.
- ``view_png_failures_r`` and ``view_pdf_failures_r`` can be used for view failures of the release build with a lag between
  opening each file.

Contributing Documentation
--------------------------

If you're browsing the documentation and notice a typo or something that could be improved, please consider letting us
know. You can either :ref:`create an issue <devdocs/contributing:Reporting a bug>` on GitHub, or click the "Edit on GitHub" button
at the top right corner of the documentation, and submit a pull request.

The GMT documentation is written in the plaintext markup language
`reStructuredText (reST) <https://docutils.sourceforge.io/rst.html>`_ and built
by documentation generator `Sphinx <https://www.sphinx-doc.org/>`__.
The reST plaintext files for the GMT documentation are located in the `doc/rst/source <https://github.com/GenericMappingTools/gmt/tree/master/doc/rst/source>`_ folder.
You may need to know some basic reST syntax before making changes. Please refer to our
:ref:`reStructuredText Cheatsheet <devdocs/rst-cheatsheet:reStructuredText Cheatsheet>` for details.

Building the documentation
~~~~~~~~~~~~~~~~~~~~~~~~~~

Usually you don't need to build the documentation locally for small changes. To build the GMT documentation you
need to `build GMT from source <https://github.com/GenericMappingTools/gmt/tree/master/BUILDING.md>`_. Be sure
to also satisfy the
`development dependencies <https://github.com/GenericMappingTools/gmt/tree/master/BUILDING.md#development-dependencies>`_
before proceeding. Have a look at the options in ``cmake/ConfigUserAdvanced.cmake`` if you want to change the
target directory for the documentation you are about to build.

To build the documentation, you also need to install some Python packages (the Sphinx theme and extensions). These packages are listed in ``doc/rst/requirements.txt`` and can be installed via::

    $ python -m pip install -r doc/rst/requirements.txt

After `configuring and building GMT from source <https://github.com/GenericMappingTools/gmt/tree/master/BUILDING.md>`_,
you can then build the GMT documentation using the following commands within the ``build`` directory::

  dvc pull
  cmake --build . --target docs_depends     # Generate images included in the documentation
  cmake --build . --target optimize_images  # Optimize PNG images for documentation [optional]
  cmake --build . --target docs_man         # Build UNIX manual pages
  cmake --build . --target docs_html        # Build HTML manual, tutorial, cookbook, and API reference

To install the UNIX manpages and html documentation into the specified location (along with the gmt executable, library, development headers and built-in data), use::

  cmake --build . --target install

.. note::
  - Refer to the file ``admin/bashrc_for_gmt`` for useful aliases for building the documentation.
  - `pngquant <https://pngquant.org/>`_ is needed for optimizing images.

Contributing an animation
~~~~~~~~~~~~~~~~~~~~~~~~~

The animations are built from the scripts in ``doc/examples/anim*/``. To add a new animation:

- Open an `issue <https://github.com/GenericMappingTools/gmt/issues>`_ with your idea for a new animation. It is best to
  get some feedback on your idea before starting work on the animation. If you do have an animation already made, you
  can share it as part of the new issue.
- Create a new script ``doc/examples/anim??/anim??.sh``, where ?? is the number of the new example. Be sure to follow the
  style of the existing animations, including using ``#!/usr/bin/env bash`` and including the purpose, list of modules
  and unix programs used, and any relevant notes. Use enough comments in your script to make it easily interpretable.
- Create a new ReStructured Text document ``doc/rst/source/animations/anim??.rst``, where ?? is the number of the new
  example. Follow the same format as the other anim??.rst files, including the ReST target ``.. _anim??:`` at the top,
  a title, and a description of the animation.
- Add a directive that will include the source code in the built documentation in ``doc/rst/source/animations/anim??.rst``::

    .. literalinclude:: /_verbatim/anim??.txt
      :language: bash

- Add a placeholder ``.. youtube::`` directive to the ``doc/rst/source/animations/anim??.rst`` file::

    ..  youtube:: Pvvc4vb8G4Y
      :width: 100%

- Add a placeholder gallery item to the end of the list of animations in ``doc/rst/source/animations.rst``::

    .. youtube:: Pvvc4vb8G4Y
      :width: 100%

      :doc:`/animations/anim??`

- :ref:`Submit a pull request <devdocs/contributing:Pull Request Workflow>` with your new animation. Please be sure
  to follow the pull request template and include the built animation in the pull request or provide a link to the built
  animation.

- If the pull request is approved, one of the GMT maintainers will build the animation, upload it to the
  `Generic Mapping Tools YouTube channel <https://www.youtube.com/channel/UCo1drOh0OZPcB7S8TmIyf8Q>`_, and update the
  links to the YouTube video in ``doc/rst/source/animations/anim??.rst`` and ``doc/rst/source/animations.rst``.

Contributing Code
-----------------

The source code for GMT is locating in the `src/ <https://github.com/GenericMappingTools/gmt/tree/master/src>`_ directory.
When contributing code, be sure to follow the general guidelines in the
:ref:`pull request workflow <devdocs/contributing:Pull Request Workflow>` section.

Code Style
~~~~~~~~~~

When modifying or submitting new source code, make sure that your code follows the GMT code style. Use the other
functions/files in the `src/ <https://github.com/GenericMappingTools/gmt/tree/master/src>`_ directory as a basis.
Here are some specific guidelines:

- Use tabs, rather than spaces, for indentation.
- Try to split lines at ~120 characters.

Testing GMT
~~~~~~~~~~~

GMT ships with more than 1000 tests to make sure that any changes won't break its functionality. In addition to the
tests located in the ``test/`` directory, GMT tests all the plots included in its documentation. The
documentation tests are located in the ``doc/scripts/`` and ``doc/examples/`` directories. The majority of GMT tests
are plot-based, with each test requiring a bash script for generating the plot and a reference PostScript file. These
tests pass if the difference between a new plot generated using the test script and the reference PostScript file is
less than a defined threshold. Other tests compute grids, tables, or other output, with the test passing if a suitable
comparison is made against a reference case.

Tests that are known to fail are excluded by adding ``# GMT_KNOWN_FAILURE`` anywhere in the test script. Tests that
require a larger tolerance than the default RMS threshold are managed using ``GRAPHICSMAGICK_RMS = <RMS>`` in the
test script. These tests are tracked in `GitHub issue #2458 <https://github.com/GenericMappingTools/gmt/issues/2458>`_.


Managing Test Images Using Data Version Control (dvc)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

As the baseline images are large blob files that can change often, it is not ideal to store them in ``git``. Instead,
we use `data version control (dvc) <https://dvc.org/>`_ to track the test images, which is like ``git`` but for data.
``dvc`` stores the hash (md5sum) of a file or an md5sum that describes the contents of a directory. For each test
``test/<module>/*.sh`` that generates a .PS file, there is a baseline image file in ``test/baseline/<module>/``
that is compared to the test result using `GraphicsMagick <http://www.graphicsmagick.org/>`_. Each of the
directories ``test/baseline/<module>`` are tracked by ``dvc`` using the file ``test/baseline/<module>.dvc``. This file
contains the hash of a JSON .dir file stored in the .dvc cache. The .dir file contains information about each tracked
file in the directory, which is used to push/pull the files to/from remote storage. The ``test/baseline/<module>.dvc``
files are stored as usual on GitHub, while the .PS files are stored separately on the ``dvc`` remote at
https://dagshub.com/GenericMappingTools/gmt.

Setting up your local environment for dvc
*****************************************

#. `Install dvc <https://dvc.org/doc/install>`_
#. If you will need to push baseline images to the remote, ask a GMT maintainer to add you as a collaborator on
   `DAGsHub <https://dagshub.com/GenericMappingTools/gmt>`_.
#. If you will need to push baseline imaged to the remote, set up
   `authentication for the DVC remote <https://dagshub.com/docs/feature_guide/dagshub_storage/#pushing-files-or-using-a-private-repo>`_.

Pulling files from the remote for testing
*****************************************

To pull or sync files from the ``dvc`` remote to your local repository, the commands are similar to ``git``:

::

    dvc status  # should report any files 'not_in_cache'
    dvc pull    # pull down files from DVC remote cache (fetch + checkout)


Once the sync is complete, you should notice that there are images stored in the ``test/baseline/<module>``
directories (e.g., ``test/baseline/api/api_matrix_as_grid.ps``). These images are technically reflinks/symlinks/copies
of the files under the ``.dvc/cache`` directory. You can now run the test suite as usual.

Running tests
^^^^^^^^^^^^^

First, pull any baseline images stored in the DAGsHub repository using dvc::

  dvc pull

After configuring CMake and building GMT, you can run all the tests by running this command in the build directory::

  cmake --build . --target check

You can also run ``ctest`` commands in the build directory. Below are some common used ctest commands.

-  Run all tests in 4 parallel jobs::

    ctest -j 4

-  Re-run all failing tests in previous run in 4 parallel jobs::

    ctest -j 4 --rerun-failed

-  Select individual tests using regexp with ctest::

    ctest --output-on-failure -R ex2[3-6]

.. note::
  Refer to the file ``admin/bashrc_for_gmt`` for useful aliases for running the tests.

Reviewing test failures
^^^^^^^^^^^^^^^^^^^^^^^

There are several tests that are "known to fail" for GMT. Unless the ``GMT_ENABLE_KNOWN2FAIL`` variable is set when
configuring CMake or setting up ``ConfigUserAdvanced.cmake``, these tests are excluded when running ctest using the
instructions provided in the :ref:`Running tests <devdocs/contributing:Running tests>` section. Therefore, you should
expect all tests to pass unless something new is broken.

Information about failing tests is produced in ``test/fail_count.txt`` inside the build directory. For plot-based tests,
the subdirectories ``test/``, ``doc/scripts/``, and ``doc/examples/`` inside the build directory contain folders for
each failing test. For plot-based tests, the directory associated with each failing tests contains a ``gmtest.sh``
script, a ``gmt.conf`` file, an alias to the test script, a PostScript file and PDF document generated by the test
script, and a PNG image that shows differences between the reference plot and new plot in magenta. In addition to these
files, running the failing tests with verbose output can be helpful for evaluating failures::

  ctest --rerun-failed --verbose

Updating reference plots for tests
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Pull requests should avoid needing to change PostScript files in the ``test/baseline/``, ``doc/scripts/images/``, and
``doc/examples/images/`` directories. However, if this is unavoidable, new PostScript reference files can be generated
using the following steps:

- Run ``dvc pull`` to ensure that you have the latest versions of the images.
- Run ``ctest -R <test-script-name>`` in the build directory after following the
  `building guide <https://github.com/GenericMappingTools/gmt/tree/master/BUILDING.md>`_, the
  :ref:`setting up your environment <devdocs/contributing:setting up your environment>` instructions, and the
  :ref:`running tests <devdocs/contributing:Running tests>` instructions.
- Copy the modified PostScript file from the appropriate subdirectory within ``build/test/``, ``build/doc/scripts/``, or
  ``build/doc/examples/`` to ``test/baseline/<module>``, ``doc/scripts/images/``, or ``doc/examples/images/`` respectively.
- Run the tests to ensure that the failing tests now pass.
- Run ``dvc diff`` to check that modified files are in the correct directory.
- Add the modified images to dvc using ``dvc add test/baseline/<module>/<newplot.ps>``, ``dvc add doc/scripts/images/<newplot.ps>``,
  or ``dvc add doc/examples/images/<newplot.ps>``
  depending on the type of test modified. RUn one dvc add command per updated PostScript plot.
- Check that the .dvc file was updated by running ``git status``.
- Stage the modified .dvc files in git using ``git add test/baseline/<module>/<newplot.ps>.dvc``, ``git add doc/scripts/images/<newplot.ps>.dvc``,
  or ``git add doc/examples/images/<newplot.ps>.dvc``, again per updated file.
- Commit the changes using ``git commit``.
- Open a pull request on GitHub with your changes.
- Push the new images to the DAGsHub repository using ``dvc push``. Optionally, use ``dvc status --remote origin`` first
  to query the diff between your local environment and the remote repository.

Adding new tests
^^^^^^^^^^^^^^^^

If you are fixing a bug or adding a new feature, you should add a test with your pull request. Most of the tests are
image based and compare a result against a reference PostScript file using `GraphicsMagick <http://www.graphicsmagick.org/>`_.

To add a PostScript based test (e.g., `box.sh <https://github.com/GenericMappingTools/gmt/blob/master/test/modern/box.sh>`_):

- Create a new shell script in the subdirectory under ``test/`` that corresponds to the module you are testing. The
  name of the shell script should be descriptive and unique.
- Include ``#!/usr/bin/env bash`` and a short description of the test at the top of the script.
- Add the content of the script that will create a PostScript file (here just called <newplot.ps>). Some general guidelines:

  - Use as small a dataset as possible. See the
    `GMT server cache <https://github.com/GenericMappingTools/gmtserver-admin/tree/master/cache>`_ for some example
    datasets that can be used.
  - Keep the script as simple as possible, with as few commands and options as needed to test the feature, enhancement,
    or bug fix.
  - Minimize the size of the resultant PostScript file as much as possible.
- Run the tests using the instructions in the :ref:`running tests <devdocs/contributing:Running tests>` section.
- Check that the new PostScript file in ``build/test/<module>`` or ``build/doc/scripts/`` is as-expected.
- Copy the new PostScript file from the appropriate subdirectory within ``build/test/``, ``build/doc/scripts/``, or
  ``build/doc/examples/`` to ``test/baseline/<module>``, ``doc/scripts/images/``, or ``doc/examples/images/`` respectively.
- Run the tests to ensure that the new test passes.
- Run ``dvc diff`` to check that the new file is in the correct directory.
- For the first test of a module, you add the directory via ``dvc add test/baseline/<module>``, ``dvc add doc/scripts/images``,
  or ``dvc add doc/scripts/examples``. For later addition you do so per file, e.g. ``dvc add test/baseline/<module>/<newplot.ps>``,
  ``dvc add doc/scripts/images/<newplot.ps>``, or ``dvc add doc/scripts/examples/<newplot.ps>``
  depending on the type of test modified.
- Check that the .dvc file was updated by running ``git status``.
- Add the modified .dvc file to git using ``git add test/baseline/<module>/<newplot.ps>.dvc``, ``git add doc/scripts/images/<newplot.ps>.dvc``, or
  ``git add doc/examples/images/<newplot.ps>.dvc``.
- Commit the changes using ``git commit``.
- Open a pull request on GitHub with your changes.
- Push the new images to the DAGsHub repository using ``dvc push``. Optionally, use ``dvc status --remote origin`` first
  to query the diff between your local environment and the remote repository.

To add a non-PostScript based test (e.g., `gmean.sh <https://github.com/GenericMappingTools/gmt/blob/master/test/blockmean/gmean.sh>`_):

- Create a new shell script in the subdirectory under ``test/`` that corresponds to the module you are testing. The
  name of the shell script should be descriptive and unique.
- Include ``#!/usr/bin/env bash`` and a short description of the test at the top of the script.
- Structure the test so that it produced both a reference file with the expected output (e.g., using ``echo`` or ``cat``)
  in a file ``answer.txt`` (for a text-based case) and the test output from gmt in a file ``result.txt``.
- Add a ``diff` command that will compare the ``result.txt`` and ``answer.txt`` files and create a file ``fail`` if the
  files do not match (e.g., ``diff -q --strip-trailing-cr answer.txt result.txt > fail``).
- Check that your new test works using the instructions in the :ref:`running tests <devdocs/contributing:Running tests>`
  section.

Debugging GMT
~~~~~~~~~~~~~

Guides for debugging GMT are provided in the :doc:`Debugging GMT </devdocs/debug>` section of the GMT documentation.
