.. GMT documentation front page

The Generic Mapping Tools Documentation
=======================================

Welcome to the GMT Docs! Here you'll find resources for using GMT and examples of what
it can do.

.. grid:: 1 1 1 1

    .. grid-item-card:: Quick Links

        .. hlist::
            :columns: 3

            - :doc:`std-opts`
            - :ref:`Projections Specifications <proj-codes>`
            - :doc:`Default Settings (gmt.conf) <gmt.conf>`
            - :doc:`Color Codes and Lists <gmtcolors>`
            - :doc:`Interactive Color Picker <color-picker>`
            - :doc:`35 Postscript Fonts </reference/postscript-fonts>`
            - :doc:`Using LaTeX in Text </reference/gmt-latex>`
            - :doc:`Built-in CPTs </reference/cpts>`
            - :doc:`Built-in Patterns </reference/predefined-patterns>`
            - :doc:`Octal Codes of Characters </reference/octal-codes>`
            - :ref:`Character Escape Sequences <Char-esc-seq>`
            - :ref:`Pen Syntax <-Wpen_attrib>`
            - :ref:`Fill Syntax <-Gfill_attrib>`
            - :ref:`Grid Format Specifications <tbl-grdformats>`
            - :doc:`theme-settings`

.. grid:: 1 2 2 2

    .. grid-item-card::

        .. toctree::
            :maxdepth: 1
            :caption: Getting started

            install
            gallery
            animations
            tutorial

    .. grid-item-card::

        .. toctree::
            :maxdepth: 1
            :caption: Reference documentation

            modules
            reference
            datasets
            modules-classic

    .. grid-item-card::

        .. toctree::
            :maxdepth: 1
            :caption: Resources

            changes
            users-contrib-symbols
            deprecated-defaults
            switching
            migrating

    .. grid-item-card::

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

.. Add a hidden toctree to suppress "document isn't included in any toctree" warnings
.. toctree::
   :hidden:

   std-opts
   gmt.conf
   gmtcolors
   color-picker
   theme-settings
