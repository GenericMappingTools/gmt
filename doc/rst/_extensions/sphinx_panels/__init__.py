""""A sphinx extension to add a ``panels`` directive."""
import hashlib
from pathlib import Path

try:
    import importlib.resources as resources
except ImportError:
    # python < 3.7
    import importlib_resources as resources

from docutils import nodes
from docutils.parsers.rst import directives, Directive
from sphinx.application import Sphinx
from sphinx.environment import BuildEnvironment
from sphinx.util.logging import getLogger

from .button import setup_link_button
from .dropdown import setup_dropdown
from .panels import setup_panels
from .tabs import setup_tabs
from .icons import setup_icons

from . import _css as css_module

__version__ = "0.5.2"

LOGGER = getLogger(__name__)


def get_default_css_variables():
    return {
        "tabs-color-label-active": "hsla(231, 99%, 66%, 1)",
        "tabs-color-label-inactive": "rgba(178, 206, 245, 0.62)",
        "tabs-color-overline": "rgb(207, 236, 238)",
        "tabs-color-underline": "rgb(207, 236, 238)",
        "tabs-size-label": "1rem",
    }


def update_css(app: Sphinx):
    """Compile SCSS to the build directory."""
    # merge user CSS variables with defaults
    css_variables = get_default_css_variables()
    for key, value in app.config.panels_css_variables.items():
        if key not in css_variables:
            LOGGER.warning(f"key in 'panels_css_variables' is not recognised: {key}")
        else:
            css_variables[key] = value

    # reset css changed attribute
    app.env.panels_css_changed = False

    # setup up new static path in output dir
    static_path = (Path(app.outdir) / "_panels_static").absolute()
    static_path.mkdir(exist_ok=True)
    app.config.html_static_path.append(str(static_path))

    # record current resources
    old_resources = {path.name for path in static_path.glob("*") if path.is_file()}

    # Add core CSS
    css_files = [r for r in resources.contents(css_module) if r.endswith(".css")]
    if app.config.panels_add_boostrap_css is not None:
        LOGGER.warning(
            "`panels_add_boostrap_css` will be deprecated. Please use"
            "`panels_add_bootstrap_css`."
        )
        app.config.panels_add_bootstrap_css = app.config.panels_add_boostrap_css

    if app.config.panels_add_bootstrap_css is False:
        css_files = [name for name in css_files if "bootstrap" not in name]
    for filename in css_files:
        app.add_css_file(filename)
        if not (static_path / filename).exists():
            content = resources.read_text(css_module, filename)
            (static_path / filename).write_text(content)
            app.env.panels_css_changed = True
        if filename in old_resources:
            old_resources.remove(filename)

    # add variables CSS file
    css_lines = [":root {"]
    for name, value in css_variables.items():
        css_lines.append(f"--{name}: {value};")
    css_lines.append("}")
    css_str = "\n".join(css_lines)
    css_variables_name = (
        f"panels-variables.{hashlib.md5(css_str.encode('utf8')).hexdigest()}.css"
    )
    app.add_css_file(css_variables_name)
    if not (static_path / css_variables_name).exists():
        (static_path / css_variables_name).write_text(css_str)
        app.env.panels_css_changed = True
    if css_variables_name in old_resources:
        old_resources.remove(css_variables_name)

    # remove old resources
    for name in old_resources:
        for path in Path(app.outdir).glob(f"**/{name}"):
            path.unlink()


def update_css_links(app: Sphinx, env: BuildEnvironment):
    """If CSS has changed, all files must be re-written,
    to include the correct stylesheets.
    """
    if env.panels_css_changed and app.config.panels_dev_mode:
        LOGGER.debug("sphinx-panels CSS changed; re-writing all files")
        return list(env.all_docs.keys())


class Div(Directive):
    """Same as the ``container`` directive,
    but does not add the ``container`` class in HTML outputs,
    which can interfere with Bootstrap CSS.
    """

    optional_arguments = 1
    final_argument_whitespace = True
    option_spec = {"name": directives.unchanged}
    has_content = True

    def run(self):
        self.assert_has_content()
        text = "\n".join(self.content)
        try:
            if self.arguments:
                classes = directives.class_option(self.arguments[0])
            else:
                classes = []
        except ValueError:
            raise self.error(
                'Invalid class attribute value for "%s" directive: "%s".'
                % (self.name, self.arguments[0])
            )
        node = nodes.container(text, is_div=True)
        node["classes"].extend(classes)
        self.add_name(node)
        self.state.nested_parse(self.content, self.content_offset, node)
        return [node]


def visit_container(self, node: nodes.Node):
    classes = "docutils container"
    if node.get("is_div", False):
        # we don't want the CSS for container for these nodes
        classes = "docutils"
    self.body.append(self.starttag(node, "div", CLASS=classes))


def depart_container(self, node: nodes.Node):
    self.body.append("</div>\n")


def setup(app: Sphinx):
    app.add_directive("div", Div)
    app.add_config_value("panels_add_bootstrap_css", None, "env")
    app.add_config_value("panels_add_boostrap_css", None, "env")
    app.add_config_value("panels_css_variables", {}, "env")
    app.add_config_value("panels_dev_mode", False, "env")
    app.connect("builder-inited", update_css)
    app.connect("env-updated", update_css_links)
    # we override container html visitors, to stop the default behaviour
    # of adding the `container` class to all nodes.container
    app.add_node(
        nodes.container, override=True, html=(visit_container, depart_container)
    )

    setup_panels(app)
    setup_link_button(app)
    setup_dropdown(app)
    setup_tabs(app)
    setup_icons(app)

    return {
        "version": __version__,
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
