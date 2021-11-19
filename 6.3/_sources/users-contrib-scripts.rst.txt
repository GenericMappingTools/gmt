.. _users_gallery:

Users Script Contributions
--------------------------

.. cssclass:: gmtgallery

.. jinja::

    {% set scripts=['vertical-slice'] %}
    {% for i in scripts %}
    -  .. figure:: /users-contrib-scripts/images/{{i}}.*
          :target: ./users-contrib-scripts/{{i}}.html

          :ref:`{{i}}`

    {% endfor %}

.. toctree::
   :hidden:
   :glob:

   users-contrib-scripts/*
