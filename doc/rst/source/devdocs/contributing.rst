Contributors Guide
==================

This is a community driven project and everyone is welcome to contribute.

The project is hosted at the `GMT GitHub repository <https://github.com/GenericMappingTools/gmt>`_.

The goal is to maintain a diverse community that's pleasant for everyone. **Please be considerate and respectful of
others**. Everyone must abide by our `Code of Conduct <https://github.com/GenericMappingTools/gmt/blob/master/CODE_OF_CONDUCT.md>`_
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
- Spread the word about GMT or start the project!

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
  `Showcase Page <https://forum.generic-mapping-tools.org/c/Sow-your-nice-example-script/>`_.

General Guidelines
------------------

Resources for New Contributors
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Please take a look at these resources to learn about Git and pull requests (don't
hesitate to :ref:`ask for help <devdocs/contributing:Getting Help>`):

- `How to Contribute to Open Source <https://opensource.guide/how-to-contribute/>`_.
- `Git Workflow Tutorial <http://www.asmeurer.com/git-workflow/) by Aaron Meurer>`_.
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

Contributing Documentation
--------------------------

If you're browsing the documentation and notice a typo or something that could be improved, please consider letting us
know. You can either :ref:`create an issue <devdocs/contributing:Reporting a bug>` on GitHub, or click the "Edit on GitHub" button
at the top right corner of the documentation, and submit a pull request.

The GMT documentation is written in the plaintext markup language
`reStructuredText (reST) <https://docutils.sourceforge.io/rst.html>`_ and built
by documentation generator `Sphinx <https://www.sphinx-doc.org/en/master/>`_.
The reST plaintext files for the GMT documentation are located in the `doc/rst/source <https://github.com/GenericMappingTools/gmt/tree/master/doc/rst/source>`_ folder.
You may need to know some basic reST syntax before making changes. Please refer to our
:ref:`reStructuredText Cheatsheet <devdocs/rst-cheatsheet:reStructuredText Cheatsheet>` for details.

Usually you don't need to build the documentation locally for small changes. If you want, you can install Sphinx
locally, then follow the instructions for `building GMT <https://github.com/GenericMappingTools/gmt/tree/master/BUILDING.md>`_
and `building the documentation <https://github.com/GenericMappingTools/gmt/tree/master/MAINTENANCE.md#building-documentation>`_.

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

Managing Test Images Using Data Version Control (dvc)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#. `Install dvc <https://dvc.org/doc/install>`_
#. If you will need to push baseline images to the remote, ask a GMT maintainer to add you as a collaborator on
   `DAGsHub <https://dagshub.com/GenericMappingTools/gmt>`_.
#. If you will need to push baseline imaged to the remote, set up
   `authentication for the DVC remote <https://dagshub.com/docs/reference/dagshub_storage/#pushing-files-or-using-a-private-repo>`_.

Pulling files from the remote for testing
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To pull or sync files from the ``dvc`` remote to your local repository, the commands are similar to ``git``:

::

    dvc status  # should report any files 'not_in_cache'
    dvc pull    # pull down files from DVC remote cache (fetch + checkout)


Once the sync is complete, you should notice that there are images stored in the ``test/baseline/<module>``
directories (e.g., ``test/baseline/api/api_matrix_as_grid.ps``). These images are technically reflinks/symlinks/copies
of the files under the ``.dvc/cache`` directory. You can now run the test suite as usual.

Migrating existing test images to dvc
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#. Sync with git and dvc remotes (``git checkout master; git pull; dvc pull``).
#. Create a branch for the module that you are working on (``git switch -c migrate-<module>-tests``).
#. Navigate to the test directory (``cd test``).
#. Remove the test image from git tracking (``git rm --cached <module>/<test-image>.ps``).
#. Create a new directory for the image in ``baseline``, if one does not already exist (``mkdir baseline/<module>``).
#. Move the test image to the new directory (``mv <module>/<test-image>.ps baseline/<module>/``).
#. Add a comment ``# DVC_TEST`` to the test script that references the image. This is used to specify that the testing
   mechanism should search in ``baseline/<module>`` for the baseline image.
#. Repeat steps 4, 6, and 7 for other .PS based tests for the module.
#. Run the GMT test suite to check that the tests work properly with the new structure.
#. Add the directory to dvc (``dvc add baseline/<module>``).
#. Commit the .dvc and modified .gitignore files to git (``git add baseline/<module>.dvc baseline/.gitignore; git commit``).
#. Push the changes to git (``git push -u origin migrate-<module>-tests``).
#. Push the changes to dvc (``dvc push``).
#. Open a pull request for the changes.