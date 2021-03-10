from urllib.parse import unquote

from docutils import nodes
from docutils.utils import unescape
from docutils.parsers.rst import directives
from sphinx import addnodes
from sphinx.util.docutils import SphinxDirective

from .utils import string_to_func_inputs


def setup_link_button(app):
    app.add_directive("link-button", LinkButton)
    # TODO hide badges in non-HTML?
    app.add_role("badge", badge_role)
    app.add_role("link-badge", link_badge_role)


def create_ref_node(link_type, uri, text, tooltip):
    innernode = nodes.inline(text, text)
    if link_type == "ref":
        ref_node = addnodes.pending_xref(
            reftarget=unquote(uri),
            reftype="any",
            # refdoc=self.env.docname,
            refdomain="",
            refexplicit=True,
            refwarn=True,
        )
        innernode["classes"] = ["xref", "any"]
        # if tooltip:
        #     ref_node["reftitle"] = tooltip
        #     ref_node["title"] = tooltip
        # TODO this doesn't work
    else:
        ref_node = nodes.reference()
        ref_node["refuri"] = uri
        if tooltip:
            ref_node["reftitle"] = tooltip
    ref_node += innernode
    return ref_node


class LinkButton(SphinxDirective):
    """A directive to turn a link into a button."""

    has_content = False
    required_arguments = 1
    final_argument_whitespace = True
    option_spec = {
        "type": lambda arg: directives.choice(arg, ("url", "ref")),
        "text": directives.unchanged,
        "tooltip": directives.unchanged,
        "classes": directives.unchanged,
    }

    def run(self):

        uri = self.arguments[0]
        link_type = self.options.get("type", "url")

        text = self.options.get("text", uri)

        ref_node = create_ref_node(
            link_type, uri, text, self.options.get("tooltip", None)
        )
        self.set_source_info(ref_node)
        ref_node["classes"] = ["sphinx-bs", "btn", "text-wrap"] + self.options.get(
            "classes", ""
        ).split()

        # sphinx requires that a reference be inside a block element
        container = nodes.paragraph()
        container += ref_node

        return [container]


def get_badge_inputs(text, cls: str = ""):
    return text, cls.split()


def badge_role(role, rawtext, text, lineno, inliner, options={}, content=[]):
    try:
        args, kwargs = string_to_func_inputs(text)
        text, classes = get_badge_inputs(*args, **kwargs)
    except Exception as err:
        msg = inliner.reporter.error(f"badge input is invalid: {err}", line=lineno)
        prb = inliner.problematic(rawtext, rawtext, msg)
        return [prb], [msg]
    node = nodes.inline(
        rawtext, unescape(text), classes=["sphinx-bs", "badge"] + classes
    )
    # textnodes, messages = inliner.parse(text, lineno, node, memo)
    # TODO this would require the memo with reporter, document and language
    return [node], []


def get_link_badge_inputs(link, text=None, type="link", cls: str = "", tooltip=None):
    return link, text or link, type, cls.split(), tooltip


def link_badge_role(role, rawtext, text, lineno, inliner, options={}, content=[]):
    try:
        args, kwargs = string_to_func_inputs(text)
        uri, text, link_type, classes, tooltip = get_link_badge_inputs(*args, **kwargs)
    except Exception as err:
        msg = inliner.reporter.error(f"badge input is invalid: {err}", line=lineno)
        prb = inliner.problematic(rawtext, rawtext, msg)
        return [prb], [msg]
    ref_node = create_ref_node(link_type, uri, text, tooltip)
    if lineno is not None:
        ref_node.source, ref_node.line = inliner.reporter.get_source_and_line(lineno)
    ref_node["classes"] = ["sphinx-bs", "badge"] + classes
    return [ref_node], []
