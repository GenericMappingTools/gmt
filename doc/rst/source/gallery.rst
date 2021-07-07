.. _example_gallery:

Example Gallery
===============

In this section we will be giving numerous examples of typical usage of
GMT programs. In general, we will start with a raw data set,
manipulate the numbers in various ways, then display the results in
diagram or map view. The resulting plots will have in common that they
are all made up of simpler plots that have been overlaid to create a
complex illustration.

.. cssclass:: gmtgallery

.. jinja:: jinja_ctx

   {% for i in range(1, 53) %}
   {% set i = '%02d' % i %}
   -  .. figure:: /_images/ex{{i}}.*
         :target: ./gallery/ex{{i}}.html

         :ref:`example_{{i}}`
   {% endfor %}

   - .. figure:: https://user-images.githubusercontent.com/14077947/123172626-62ca6880-d44b-11eb-8d91-21e448930460.png
         :target: ./gallery/ex53.html

         :ref:`example_53`

.. toctree::
   :hidden:
   :glob:

   gallery/ex*
