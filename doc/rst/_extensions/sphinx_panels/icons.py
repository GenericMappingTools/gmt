from functools import lru_cache
import json
from pathlib import Path

from docutils import nodes

from .utils import string_to_func_inputs


OPTICON_VERSION = "0.0.0-dd899ea"

OPTICON_CSS = """\
.octicon {
  display: inline-block;
  vertical-align: text-top;
  fill: currentColor;
}"""


@lru_cache(1)
def get_opticon_data():
    path = Path(__file__).parent.joinpath("data", "opticons.json")
    return json.loads(path.read_text())


def list_opticons():
    return list(get_opticon_data().keys())


def get_opticon(
    name: str,
    classes: str = None,
    width: int = None,
    height: int = None,
    aria_label: str = None,
    size: int = 16,
):
    assert size in [16, 24], "size must be 16 or 24"
    try:
        data = get_opticon_data()[name]
    except KeyError:
        raise KeyError(f"Unrecognised opticon: {name}")

    content = data["heights"][str(size)]["path"]
    options = {
        "version": "1.1",
        "width": data["heights"][str(size)]["width"],
        "height": int(size),
        "class": f"octicon octicon-{name}",
    }

    if width is not None or height is not None:
        if width is None:
            width = round((int(height) * options["width"]) / options["height"], 2)
        if height is None:
            height = round((int(width) * options["height"]) / options["width"], 2)
        options["width"] = width
        options["height"] = height

    options["viewBox"] = f'0 0 {options["width"]} {options["height"]}'

    if classes is not None:
        options["class"] += " " + classes.strip()

    if aria_label is not None:
        options["aria-label"] = aria_label
        options["role"] = "img"
    else:
        options["aria-hidden"] = "true"

    opt_string = " ".join(f'{k}="{v}"' for k, v in options.items())
    return f"<svg {opt_string}>{content}</svg>"


def opticon_role(
    role, rawtext: str, text: str, lineno, inliner, options={}, content=[]
):
    try:
        args, kwargs = string_to_func_inputs(text)
        svg = get_opticon(*args, **kwargs)
    except Exception as err:
        msg = inliner.reporter.error(f"Opticon input is invalid: {err}", line=lineno)
        prb = inliner.problematic(rawtext, rawtext, msg)
        return [prb], [msg]
    node = nodes.raw("", nodes.Text(svg), format="html")
    return [node], []


class fontawesome(nodes.Element, nodes.General):
    pass


def create_fa_node(name, classes: str = None, style="fa"):
    assert style.startswith("fa"), "style must be a valid prefix, e.g. fa, fas, etc"
    return fontawesome(
        icon_name=name,
        classes=[style, f"fa-{name}"] + (classes.split() if classes else []),
    )


def fontawesome_role(role, rawtext, text, lineno, inliner, options={}, content=[]):
    try:
        args, kwargs = string_to_func_inputs(text)
        node = create_fa_node(*args, **kwargs)
    except Exception as err:
        msg = inliner.reporter.error(
            f"FontAwesome input is invalid: {err}", line=lineno
        )
        prb = inliner.problematic(rawtext, rawtext, msg)
        return [prb], [msg]
    return [node], []


def visit_fontawesome_html(self, node):
    self.body.append(self.starttag(node, "span", ""))


def depart_fontawesome_html(self, node):
    self.body.append("</span>")


def visit_fontawesome_latex(self, node):
    if self.config.panels_add_fontawesome_latex:
        self.body.append(f"\\faicon{{{node['icon_name']}}}")
    raise nodes.SkipNode


def add_fontawesome_pkg(app, config):
    if app.config.panels_add_fontawesome_latex:
        app.add_latex_package("fontawesome")


def setup_icons(app):
    app.add_role("opticon", opticon_role)
    app.add_role("fa", fontawesome_role)

    app.add_config_value("panels_add_fontawesome_latex", False, "env")
    app.connect("config-inited", add_fontawesome_pkg)
    app.add_node(
        fontawesome,
        html=(visit_fontawesome_html, depart_fontawesome_html),
        latex=(visit_fontawesome_latex, None),
        text=(None, None),
        man=(None, None),
        texinfo=(None, None),
    )
