// Copyright 2013 PSF. Licensed under the PYTHON SOFTWARE FOUNDATION LICENSE VERSION 2
// File originates from the cpython source found in Doc/tools/sphinxext/static/version_switch.js

(function() {
  'use strict';

  var doc_url = "docs.generic-mapping-tools.org";
  // var doc_url = "0.0.0.0:8000"; // for local testing only
  var url_re = new RegExp(doc_url + "\\/(dev|latest|(\\d+\\.\\d+))\\/");
  // List all versions.
  // Add one entry "version: title" for any minor releases
  var all_versions = {
    'latest': 'latest',
    'dev': 'dev',
    '6.1': '6.1',
    '6.0': '6.0',
    '5.4': '5.4',
    '4.5': '4',
  };

  function build_select(current_version, current_release) {
    var buf = ['<select>'];

    $.each(all_versions, function(version, title) {
      buf.push('<option value="' + version + '"');
      if (version == current_version) {
        buf.push(' selected="selected">');
        if (version == "latest" || version == "dev") {
          buf.push(title + ' (' + current_release + ')');
        } else {
          buf.push(current_version);
        }
      } else {
        buf.push('>' + title);
      }
      buf.push('</option>');
    });

    buf.push('</select>');
    return buf.join('');
  }

  function patch_url(url, new_version) {
    return url.replace(url_re, doc_url + '/' + new_version + '/');
  }

  function on_switch() {
    var selected = $(this).children('option:selected').attr('value');

    var url = window.location.href,
        new_url = patch_url(url, selected);
    console.log("OK");
    console.log(new_url);
    console.log(url);

    if (new_url != url) {
      // check beforehand if url exists, else redirect to version's start page
      $.ajax({
        url: new_url,
        success: function() {
           window.location.href = new_url;
        },
        error: function() {
            window.location.href = 'http://' + doc_url + '/' + selected;
        }
      });
    }
  }

  $(document).ready(function() {
    var match = url_re.exec(window.location.href);
    if (match) {
      var release = DOCUMENTATION_OPTIONS.VERSION;
      var version = match[1];
      var select = build_select(version, release);
      $('.version_switch_note').html('Or, select a version from the drop-down menu above.');
      $('.version').html(select);
      $('.version select').bind('change', on_switch);
    }
  });
})();
