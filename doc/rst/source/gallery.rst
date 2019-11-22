.. _example_gallery:

Example Gallery
===============

The 50 Examples
----------------

In this section we will be giving numerous examples of typical usage of
GMT programs. In general, we will start with a raw data set,
manipulate the numbers in various ways, then display the results in
diagram or map view. The resulting plots will have in common that they
are all made up of simpler plots that have been overlaid to create a
complex illustration.

.. cssclass:: gmtgallary

.. jinja::

   {% for i in range(1, 51) %}
   {% set i = '%02d' % i %}
   -  .. figure:: /_images/ex{{i}}.*
         :target: ./gallery/ex{{i}}.html

         :ref:`example_{{i}}`
   {% endfor %}

.. toctree::
   :hidden:
   :glob:

   gallery/ex*

Animations
----------

.. cssclass:: gmtgallary

.. jinja::

    {% for i in range(1, 9) %}
    {% set i = '%02d' % i %}
    -  .. figure:: /_images/anim{{i}}.*
          :target: ./gallery/anim{{i}}.html

          :ref:`anim{{i}}`

    {% endfor %}

.. toctree::
   :hidden:
   :glob:

   gallery/anim_introduction.rst
   gallery/anim*
