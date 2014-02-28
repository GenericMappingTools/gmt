/*
 * rtd-sidebar.js
 * ~~~~~~~~~~~~~~
 *
 * This script makes the Sphinx sidebar collapsible.
 *
 * .sphinxsidebar contains .sphinxsidebarwrapper.  This script adds
 * in .sphixsidebar, after .sphinxsidebarwrapper, the #sidebarbutton
 * used to collapse and expand the sidebar.
 *
 * When the sidebar is collapsed the .sphinxsidebarwrapper is hidden
 * and the width of the sidebar and the margin-left of the document
 * are decreased. When the sidebar is expanded the opposite happens.
 * This script saves a per-browser/per-session cookie used to
 * remember the position of the sidebar among the pages.
 * Once the browser is closed the cookie is deleted and the position
 * reset to the default (expanded).
 *
 * Adapted from sidebar.js to be used with RTD theme.
 * Copyright 2007-2011 by the Sphinx team.
 * License BSD.
 *
 */

$(function() {
    // global elements used by the functions.
    // the 'sidebarbutton' element is defined as global after its
    // creation, in the add_sidebar_button function
    var bodywrapper = $('.bodywrapper');
    var sidebar = $('.sphinxsidebar');
    var sidebarwrapper = $('.sphinxsidebarwrapper');

    // for some reason, the document has no sidebar; do not run into errors
    if (!sidebar.length) return;

    // original margin-left of the bodywrapper and width of the sidebar
    // with the sidebar expanded
    var bw_margin_expanded = bodywrapper.css('margin-left');
    var ssb_padding_expanded = sidebar.css('padding');
    var ssb_width_expanded = sidebar.width();

    // margin-left of the bodywrapper and width of the sidebar
    // with the sidebar collapsed
    var bw_margin_collapsed = '.9em';
    var ssb_width_collapsed = '.7em';

    // colors used by the current theme
    var dark_color = $('h3').css('color');
    var light_color = $('.sphinxsidebar').css('background-color');

    function sidebar_is_collapsed() {
      return sidebarwrapper.is(':not(:visible)');
    }

    function toggle_sidebar() {
      if (sidebar_is_collapsed())
        expand_sidebar();
      else
        collapse_sidebar();
    }

    function collapse_sidebar() {
      sidebarwrapper.hide();
      sidebar.css({
          'width': ssb_width_collapsed,
          'padding': '0px'
          });
      bodywrapper.css('margin-left', bw_margin_collapsed);
      sidebarbutton.css({
          'margin-left': '0',
          'height': bodywrapper.height()
          });
      sidebarbutton.find('span').text('»');
      sidebarbutton.attr('title', _('Expand sidebar'));
      document.cookie = 'sidebar=collapsed';
    }

    function expand_sidebar() {
      bodywrapper.css('margin-left', bw_margin_expanded);
      sidebar.css({
          'padding': ssb_padding_expanded,
          'width': ssb_width_expanded
          });
      sidebarwrapper.show();
      sidebarbutton.css({
          'margin-left': ssb_width_expanded,
          'height': bodywrapper.height()
          });
      sidebarbutton.find('span').text('«');
      sidebarbutton.attr('title', _('Collapse sidebar'));
      document.cookie = 'sidebar=expanded';
    }

    function add_sidebar_button() {
      sidebarwrapper.css({
          'float': 'left',
          'margin-right': '0',
          'width': ssb_width_expanded - 3
          });
      // create the button
      sidebar.append(
          '<div id="sidebarbutton"><span>&laquo;</span></div>'
          );
      var sidebarbutton = $('#sidebarbutton');
      light_color = sidebarbutton.css('background-color');
      // find the height of the viewport to center the '<<' in the page
      var viewport_height;
      var document_height = $('.document').height();
      if (window.innerHeight)
        viewport_height = window.innerHeight;
      else
        viewport_height = $(window).height();
      if (document_height < viewport_height) // use the smaller one
        viewport_height = document_height;
      sidebarbutton.find('span').css({
          'display': 'block',
          'margin-top': (viewport_height - sidebar.position().top - 20) / 2
          });

      sidebarbutton.click(toggle_sidebar);
      sidebarbutton.attr('title', _('Collapse sidebar'));
      sidebarbutton.css({
          'color': '#444444',
          'border-left': 'solid transparent',
          'font-size': '1.2em',
          'cursor': 'pointer',
          'height': bodywrapper.height(),
          'padding-top': '1px',
          'padding-right': ssb_width_collapsed,
          'margin-left': ssb_width_expanded
          });

      sidebarbutton.hover(
          function () {
          $(this).css('background-color', dark_color);
          },
          function () {
          $(this).css('background-color', light_color);
          }
          );
    }

    function set_position_from_cookie() {
      if (!document.cookie)
        return sidebar_collapse_on_small_screen();
      var items = document.cookie.split(';');
      for(var k=0; k<items.length; k++) {
        var key_val = items[k].split('=');
        var key = key_val[0];
        if (key == 'sidebar') {
          var value = key_val[1];
          if ((value == 'collapsed') && (!sidebar_is_collapsed()))
            collapse_sidebar();
          else if ((value == 'expanded') && (sidebar_is_collapsed()))
            expand_sidebar();
        }
      }
    }

    function sidebar_collapse_on_small_screen() {
      // auto-collapse sidebar on small screens
      var viewport_width;
      if (window.innerWidth)
        viewport_width = window.innerWidth;
      else
        viewport_width = $(window).width();
      if (viewport_width < 700)
        collapse_sidebar();
    }

    add_sidebar_button();
    var sidebarbutton = $('#sidebarbutton');
    set_position_from_cookie();
});
