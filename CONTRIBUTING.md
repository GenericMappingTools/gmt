# Contributing Guidelines

**First of all, thank you for considering contributing to the project.**

This is a community-driven project, so it's people like you that make it useful and
successful.
These are some of the many ways to contribute:

* :bug: Submitting bug reports and feature requests
* :memo: Writing tutorials or examples
* :mag: Fixing typos and improving to the documentation
* :bulb: Writing code for everyone to use

If you get stuck at any point you can create an issue on GitHub (look for the *Issues*
tab in the repository) or contact us at one of the other channels mentioned below.

For more information on contributing to open source projects,
[GitHub's own guide](https://guides.github.com/activities/contributing-to-open-source/)
is a great starting point if you are new to version control.


## Ground Rules

The goal is to maintain a diverse community that's pleasant for everyone.
**Please be considerate and respectful of others**.
Everyone must abide by our [Code of Conduct](CODE_OF_CONDUCT.md) and we encourage all to
read it carefully.


## Contents

* [What Can I Do?](#what-can-i-do)
* [How Can I Talk to You?](#how-can-i-talk-to-you)
* [Reporting a Bug](#reporting-a-bug)
* [Editing the Documentation](#editing-the-documentation)
* [Contributing Code](#contributing-code)
  - [General guidelines](#general-guidelines)
  - [Code Review](#code-review)


## What Can I Do?

* Tackle any [issue](https://github.com/GenericMappingTools/gmt/issues) that you wish!
  Please leave a comment on the issue indicating that you want to work on it.
  Some issues are labeled as
  ["good first issues"](https://github.com/GenericMappingTools/gmt/issues?q=is%3Aopen+is%3Aissue+label%3A%22good+first+issue%22)
  to indicate that they are beginner friendly, meaning that they don't require extensive
  knowledge of the project.
* Report a bug you found through the [Github issues](https://github.com/GenericMappingTools/gmt/issues).
* Make a tutorial or example of how to do something.
* Provide feedback about how we can improve the project or about your particular use
  case.
* Contribute code you already have. It doesn't need to be perfect! We will help you
  clean things up, test it, etc.


## How Can I Talk to You?

Discussion often happens in the issues and pull requests.
For general questions, you can post on the
[GMT Community Forum](https://forum.generic-mapping-tools.org/).


## Reporting a Bug

Find the [Issues](https://github.com/GenericMappingTools/gmt/issues) tab on the top of
the Github repository and click *New Issue*.
You'll be prompted to choose between different types of issue, like bug reports and
feature requests.
Choose the one that best matches your need.
The Issue will be populated with one of our templates.
**Please try to fillout the template with as much detail as you can**.
Remember: the more information we have, the easier it will be for us to solve your
problem.


## Editing the Documentation

If you're browsing the documentation and notice a typo or something that could be
improved, please consider letting us know. You can either
[create an issue](#reporting-a-bug) on GitHub, or click the "Edit on GitHub" button
at the top right corner of the documentation, and submit a fix (even better :star2:).

The GMT documentation is written in the plaintext markup language
[reStructuredText](https://docutils.sourceforge.io/rst.html) and built
by documentation generator [Sphinx](https://www.sphinx-doc.org/en/master/).
To build the documentation locally, you need to have Sphinx installed,
then follow the [build instructions](MAINTENANCE.md#building-documentation).
For more details about the reStructuredText markup language, please refer to our
[reStructuredText Cheatsheet](https://docs.generic-mapping-tools.org/latest/rst_cheatsheet.html).


## Contributing Code

**Is this your first contribution?**
Please take a look at these resources to learn about git and pull requests (don't
hesitate to [ask questions](#how-can-i-talk-to-you)):

* [How to Contribute to Open Source](https://opensource.guide/how-to-contribute/).
* Aaron Meurer's [tutorial on the git workflow](http://www.asmeurer.com/git-workflow/)
* [How to Contribute to an Open Source Project on GitHub](https://egghead.io/courses/how-to-contribute-to-an-open-source-project-on-github)

### General guidelines

We follow the [git pull request workflow](http://www.asmeurer.com/git-workflow/) to
make changes to our codebase.
Every change made goes through a pull request, even our own, so that our
[continuous integration](https://en.wikipedia.org/wiki/Continuous_integration) services
have a change to check that the code is up to standards and passes all our tests.
This way, the *master* branch is always stable.

General guidelines for pull requests (PRs):

* **Open an issue first** describing what you want to do. If there is already an issue
  that matches your PR, leave a comment there instead to let us know what you plan to
  do.
* Each pull request should consist of a **small** and logical collection of changes.
* Larger changes should be broken down into smaller components and integrated
  separately.
* Bug fixes should be submitted in separate PRs.
* Describe what your PR changes and *why* this is a good thing. Be as specific as you
  can. The PR description is how we keep track of the changes made to the project over
  time.
* Do not commit changes to files that are irrelevant to your feature or bugfix (eg:
  `.gitignore`, IDE project files, etc).
* Write descriptive commit messages. Chris Beams has written a
  [guide](https://chris.beams.io/posts/git-commit/) on how to write good commit
  messages.
* Be willing to accept criticism and work on improving your code; we don't want to break
  other users' code, so care must be taken not to introduce bugs.
* Be aware that the pull request review process is not immediate, and is generally
  proportional to the size of the pull request.

### Code Review

After you've submitted a pull request, you should expect to hear at least a comment
within a couple of days.
We may suggest some changes or improvements or alternatives.

Some things that will increase the chance that your pull request is accepted quickly:

* Write a good and detailed description of what the PR does.
* Write tests for the code you wrote/modified.
* Readable code is better than clever code (even with comments).
* Write documentation for your code and leave comments explaining the *reason* behind
  non-obvious things.
* Include an example of new features in the gallery or tutorials.

Pull requests will automatically have tests run by Azure Pipelines.
Github will show the status of these checks on the pull request.
Try to get them all passing (green).
If you have any trouble, leave a comment in the PR or
[get in touch](#how-can-i-talk-to-you).
