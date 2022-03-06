from uuid import uuid4
from typing import Optional

from docutils import nodes
from docutils.parsers.rst import directives
from sphinx.transforms.post_transforms import SphinxPostTransform
from sphinx.util.docutils import SphinxDirective
from sphinx.util.logging import getLogger
from sphinx.util.nodes import NodeMatcher

LOGGER = getLogger(__name__)


def setup_tabs(app):
    app.add_directive("tabbed", TabbedDirective)
    app.add_post_transform(TabbedHtmlTransform)
    app.add_node(tabbed_input, html=(visit_tabbed_input, depart_tabbed_input))
    app.add_node(tabbed_label, html=(visit_tabbed_label, depart_tabbed_label))


class tabbed_input(nodes.Element, nodes.General):
    pass


class tabbed_label(nodes.TextElement, nodes.General):
    pass


def visit_tabbed_input(self, node):
    attributes = {"ids": [node["id"]], "type": node["type"], "name": node["set_id"]}
    if node["checked"]:
        attributes["checked"] = "checked"
    self.body.append(self.starttag(node, "input", **attributes))


def depart_tabbed_input(self, node):
    self.body.append("</input>")


def visit_tabbed_label(self, node):
    attributes = {"for": node["input_id"]}
    self.body.append(self.starttag(node, "label", **attributes))


def depart_tabbed_label(self, node):
    self.body.append("</label>")


class TabbedDirective(SphinxDirective):
    """CSS-based tabs."""

    required_arguments = 1
    final_argument_whitespace = True
    has_content = True
    option_spec = {
        "new-group": directives.flag,
        "selected": directives.flag,
        "name": directives.unchanged,
        "class-label": directives.class_option,
        "class-content": directives.class_option,
    }

    def run(self):
        self.assert_has_content()

        container = nodes.container(
            "",
            type="tabbed",
            new_group="new-group" in self.options,
            selected="selected" in self.options,
            classes=["tabbed-container"],
        )
        self.set_source_info(container)

        # add label as a rubric (to degrade nicely for non-html outputs)
        textnodes, messages = self.state.inline_text(self.arguments[0], self.lineno)
        label = nodes.rubric(self.arguments[0], *textnodes, classes=["tabbed-label"])
        label["classes"] += self.options.get("class-label", [])
        self.add_name(label)
        container += label

        # add content
        content = nodes.container("", is_div=True, classes=["tabbed-content"])
        content["classes"] += self.options.get("class-content", [])
        self.state.nested_parse(self.content, self.content_offset, content)

        container += content

        return [container]


class TabSet:
    def __init__(self, node):
        self._nodes = [node]

    def is_next(self, node):
        if self.parent != node.parent:
            return False
        if node.parent.index(node) != (self.indices[-1] + 1):
            return False
        return True

    def append(self, node):
        assert self.is_next(node)
        self._nodes.append(node)

    @property
    def parent(self) -> int:
        return self._nodes[0].parent

    @property
    def nodes(self) -> int:
        return self._nodes[:]

    @property
    def indices(self) -> int:
        return [n.parent.index(n) for n in self._nodes]


class TabbedHtmlTransform(SphinxPostTransform):
    default_priority = 200
    builders = ("html", "dirhtml", "singlehtml", "readthedocs")

    def get_unique_key(self):
        return str(uuid4())

    def run(self):
        matcher = NodeMatcher(nodes.container, type="tabbed")
        tab_set = None
        for node in self.document.traverse(matcher):  # type: nodes.container
            if tab_set is None:
                tab_set = TabSet(node)
            elif node["new_group"]:
                self.render_tab_set(tab_set)
                tab_set = TabSet(node)
            elif tab_set.is_next(node):
                tab_set.append(node)
            else:
                self.render_tab_set(tab_set)
                tab_set = TabSet(node)
        self.render_tab_set(tab_set)

    def render_tab_set(self, tab_set: Optional[TabSet]):

        if tab_set is None:
            return

        container = nodes.container("", is_div=True, classes=["tabbed-set"])
        container.parent = tab_set.parent
        set_identity = self.get_unique_key()

        # get the first selected node
        selected_idx = None
        for idx, tab in enumerate(tab_set.nodes):
            if tab["selected"]:
                if selected_idx is None:
                    selected_idx = idx
                else:
                    LOGGER.warning("multiple selected tabbed directives", location=tab)
        selected_idx = 0 if selected_idx is None else selected_idx

        for idx, tab in enumerate(tab_set.nodes):
            # TODO warn and continue if incorrect children
            title, content = tab.children
            # input <input checked="checked" id="id" type="radio">
            identity = self.get_unique_key()
            input_node = tabbed_input(
                "",
                id=identity,
                set_id=set_identity,
                type="radio",
                checked=(idx == selected_idx),
            )
            input_node.source, input_node.line = tab.source, tab.line
            container += input_node
            # label <label for="id">Title</label>
            # TODO this actually has to be text only
            label = tabbed_label("", *title.children, input_id=identity)
            label["classes"] = title["classes"]
            container += label
            input_node.source, input_node.line = tab.source, tab.line
            # content
            container += content

        # replace all nodes
        tab_set.parent.children = (
            tab_set.parent.children[: tab_set.indices[0]]
            + [container]
            + tab_set.parent.children[tab_set.indices[-1] + 1 :]
        )
