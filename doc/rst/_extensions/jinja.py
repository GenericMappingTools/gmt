#
# sphinx-jinja v2.0.2 is available from https://github.com/tardyp/sphinx-jinja
# licensed under the MIT License.
#
# MIT License

# Copyright (c) 2016-2021 Pierre Tardy and contributors

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


import codecs
import os
import sys

from docutils import nodes
from docutils.parsers.rst import Directive
from docutils.parsers.rst import directives
from docutils.statemachine import StringList
from jinja2 import FileSystemLoader, Environment
import sphinx.util

try:
    from urllib.request import url2pathname
except ImportError:
    from urllib import url2pathname


class JinjaDirective(Directive):
    has_content = True
    optional_arguments = 1
    option_spec = {
        "file": directives.path,
        "header_char": directives.unchanged,
        "header_update_levels": directives.flag,
        "debug": directives.unchanged,
    }
    app = None

    def run(self):
        node = nodes.Element()
        node.document = self.state.document
        docname = self.state.document.settings.env.docname
        conf = self.app.config
        template_filename = self.options.get("file")
        debug_template = self.options.get("debug")
        cxt = (conf.jinja_contexts[self.arguments[0]].copy()
               if self.arguments else {})
        cxt["options"] = {
            "header_char": self.options.get("header_char"),
        }
        env = Environment(
            loader=FileSystemLoader(conf.jinja_base, followlinks=True),
            **conf.jinja_env_kwargs
        )
        env.filters.update(conf.jinja_filters)
        env.tests.update(conf.jinja_tests)
        env.globals.update(conf.jinja_globals)
        env.policies.update(conf.jinja_policies)
        if template_filename:
            if debug_template is not None:
                reference_uri = directives.uri(template_filename)
                template_path = os.path.join(
                    os.path.abspath(conf.jinja_base),
                    url2pathname(reference_uri),
                )
                encoded_path = template_path.encode(sys.getfilesystemencoding())
                with codecs.open(encoded_path, encoding='utf-8') as f:
                    debug_print(
                        'Template Before Processing',
                        '******* From {} *******\n{}'.format(docname, f.read()),
                    )
            tpl = env.get_template(template_filename)
        else:
            content = '\n'.join(self.content)
            if debug_template is not None:
                debug_print('Template Before Processing', content)
            tpl = env.from_string(content)
        new_content = tpl.render(**cxt)
        if debug_template is not None:
            debug_print('Template After Processing', new_content)

        if "header_update_levels" in self.options:
            self.state_machine.insert_input(new_content.splitlines(), source='')
            return []
        else:
            new_content = StringList(new_content.splitlines(), source='')
            sphinx.util.nested_parse_with_titles(self.state, new_content, node)
            return node.children


def debug_print(title, content):
    stars = '*' * 10
    print('\n{1} Begin Debug Output: {0} {1}'.format(title, stars))
    print(content)
    print('\n{1} End Debug Output: {0} {1}'.format(title, stars))


def setup(app):
    JinjaDirective.app = app
    app.add_directive('jinja', JinjaDirective)
    app.add_config_value('jinja_contexts', {}, 'env')
    app.add_config_value('jinja_base', app.srcdir, 'env')
    app.add_config_value('jinja_env_kwargs', {}, 'env')
    app.add_config_value('jinja_filters', {}, 'env')
    app.add_config_value('jinja_tests', {}, 'env')
    app.add_config_value('jinja_globals', {}, 'env')
    app.add_config_value('jinja_policies', {}, 'env')
    return {'parallel_read_safe': True, 'parallel_write_safe': True}
