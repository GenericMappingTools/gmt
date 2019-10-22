#
# The sphinx-jinja extension is available from https://github.com/tardyp/sphinx-jinja,
# licensed under the MIT License.
#
#
# The MIT License
#
# Copyright (c) 2016-2019 Pierre Tardy
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.


import codecs
import os
import sys
import urllib

from docutils import nodes
from docutils.parsers.rst import Directive
from docutils.parsers.rst import directives
from docutils.statemachine import StringList
from jinja2 import FileSystemLoader, Environment
import sphinx.util


class JinjaDirective(Directive):
    has_content = True
    optional_arguments = 1
    option_spec = {
        "file": directives.path,
        "header_char": directives.unchanged,
        "debug": directives.unchanged,
    }
    app = None

    def run(self):
        node = nodes.Element()
        node.document = self.state.document
        env = self.state.document.settings.env
        docname = env.docname
        template_filename = self.options.get("file")
        debug_template = self.options.get("debug")
        cxt = (self.app.config.jinja_contexts[self.arguments[0]].copy()
               if self.arguments else {})
        cxt["options"] = {
            "header_char": self.options.get("header_char")
        }
        if template_filename:
            if debug_template is not None:
                print('')
                print('********** Begin Jinja Debug Output: Template Before Processing **********')
                print('********** From {} **********'.format(docname))
                reference_uri = directives.uri(os.path.join('source', template_filename))
                template_path = urllib.url2pathname(reference_uri)
                encoded_path = template_path.encode(sys.getfilesystemencoding())
                imagerealpath = os.path.abspath(encoded_path)
                with codecs.open(imagerealpath, encoding='utf-8') as f:
                    print(f.read())
                print('********** End Jinja Debug Output: Template Before Processing **********')
                print('')
            tpl = Environment(
                          loader=FileSystemLoader(
                              self.app.config.jinja_base, followlinks=True)
                      ).get_template(template_filename)
        else:
            if debug_template is not None:
                print('')
                print('********** Begin Jinja Debug Output: Template Before Processing **********')
                print('********** From {} **********'.format(docname))
                print('\n'.join(self.content))
                print('********** End Jinja Debug Output: Template Before Processing **********')
                print('')
            tpl = Environment(
                      loader=FileSystemLoader(
                          self.app.config.jinja_base, followlinks=True)
                  ).from_string('\n'.join(self.content))
        new_content = tpl.render(**cxt)
        if debug_template is not None:
            print('')
            print('********** Begin Jinja Debug Output: Template After Processing **********')
            print(new_content)
            print('********** End Jinja Debug Output: Template After Processing **********')
            print('')
        new_content = StringList(new_content.splitlines(), source='')
        sphinx.util.nested_parse_with_titles(
            self.state, new_content, node)
        return node.children


def setup(app):
    JinjaDirective.app = app
    app.add_directive('jinja', JinjaDirective)
    app.add_config_value('jinja_contexts', {}, 'env')
    app.add_config_value('jinja_base', os.path.abspath('.'), 'env')
    return {'parallel_read_safe': True, 'parallel_write_safe': True}
