.. GMT documentation front page

The Generic Mapping Tools Documentation
=======================================

Welcome to the GMT Docs! Here you'll find resources for using GMT and examples of what
it can do.

Quick links
-----------

.. hlist::
   :columns: 3

   - :doc:`std-opts`
   - :ref:`Projections Specifications <proj-codes>`
   - :doc:`Default Settings (gmt.conf) <gmt.conf>`
   - :doc:`Colors <gmtcolors>`
   - :doc:`35 Postscript Fonts </cookbook/postscript-fonts>`
   - :doc:`Using LaTeX in text </cookbook/gmt-latex>`
   - :doc:`Built-in CPTs </cookbook/cpts>`
   - :doc:`Built-in patterns </cookbook/predefined-patterns>`
   - :doc:`Octal Codes of Characters </cookbook/octal-codes>`
   - :ref:`Character Escape Sequences <Char-esc-seq>`
   - :ref:`Pen Syntax <-Wpen_attrib>`
   - :ref:`Fill Syntax <-Gfill_attrib>`
   - :ref:`Grid Format Specifications <tbl-grdformats>`
   - :doc:`theme-settings`

.. Add a hidden toctree to suppress "document isn't included in any toctree" warnings
.. toctree::
   :hidden:

   std-opts
   gmt.conf
   gmtcolors
   theme-settings

.. panels::

    .. toctree::
        :maxdepth: 1
        :caption: Getting started

        gallery
        animations
        tutorial
        tutorial_jl
        Tutorials in PyGMT <https://www.pygmt.org/latest/tutorials/index.html>

    ---

    .. toctree::
        :maxdepth: 1
        :caption: Reference documentation

        modules
        cookbook
        datasets

    ---

    .. toctree::
        :maxdepth: 1
        :caption: Resources

        changes
        users-contrib-scripts
        users-contrib-symbols
        deprecated-defaults
        switching
        migrating

    ---

    .. toctree::
        :maxdepth: 1
        :caption: Classic Mode

        std-opts-classic
        modules-classic

    ---

    .. toctree::
        :maxdepth: 1
        :caption: Development

        Code of Conduct <https://github.com/GenericMappingTools/.github/blob/main/CODE_OF_CONDUCT.md>
        /devdocs/contributing
        /devdocs/maintenance
        /devdocs/team
        reStructuredText Cheatsheet </devdocs/rst-cheatsheet>
        Debugging GMT </devdocs/debug>
        GMT C API </devdocs/api>
        PostScriptLight C API </devdocs/postscriptlight>
        /devdocs/devdocs
