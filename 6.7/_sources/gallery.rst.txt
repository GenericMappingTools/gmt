:orphan:

.. _example_gallery:

Illustration Gallery
====================

In this section we will be giving numerous examples of typical usage of
GMT programs. In general, we will start with a raw data set,
manipulate the numbers in various ways, then display the results in
diagram or map view. The resulting plots will have in common that they
are all made up of simpler plots that have been overlaid to create a
complex illustration.

.. grid:: 2 3 3 4

    .. jinja::

        {% for i in range(1, 53) %}
        {% set i = '%02d' % i %}
        .. grid-item-card:: :doc:`gallery/ex{{i}}`
            :padding: 1
            :link-type: doc
            :link: gallery/ex{{i}}

            .. figure:: /_images/ex{{i}}.*
        {% endfor %}

    .. grid-item-card:: :ref:`example_53`
        :padding: 1
        :link-type: doc
        :link: gallery/ex53

        .. figure:: https://user-images.githubusercontent.com/14077947/123172626-62ca6880-d44b-11eb-8d91-21e448930460.png

.. toctree::
   :hidden:
   :glob:

   gallery/ex*
