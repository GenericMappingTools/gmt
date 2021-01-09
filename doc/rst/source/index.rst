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
   - :doc:`proj-codes`
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

.. Add a hidden toctree to suppress "document isn't included in any toctree" warnings
.. toctree::
   :hidden:

   std-opts
   proj-codes
   gmt.conf
   gmtcolors

.. toctree::
   :maxdepth: 1
   :caption: Getting started

   gallery
   animations
   tutorial

.. toctree::
   :maxdepth: 1
   :caption: Reference documentation

   modules
   cookbook
   datasets

.. toctree::
   :maxdepth: 1
   :caption: Resources

   users-contrib-scripts
   users-contrib-symbols
   changes
   deprecated-defaults
   switching
   migrating

.. toctree::
   :maxdepth: 1
   :caption: Classic Mode

   std-opts-classic
   modules-classic

.. toctree::
   :maxdepth: 1
   :caption: Developer Resources

   Contributing Guide <https://github.com/GenericMappingTools/gmt/blob/master/CONTRIBUTING.md>
   Code of Conduct <https://github.com/GenericMappingTools/gmt/blob/master/CODE_OF_CONDUCT.md>
   reStructuredText Cheatsheet <rst-cheatsheet>
   Debugging GMT <debug>
   GMT C API <api>
   PostScriptLight C API <postscriptlight>
   devdocs
