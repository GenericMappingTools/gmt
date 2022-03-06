"""Originally Adapted from sphinxcontrib.details.directive
"""
from docutils import nodes
from docutils.parsers.rst import directives
from sphinx.util.docutils import SphinxDirective
from sphinx.transforms.post_transforms import SphinxPostTransform
from sphinx.util.nodes import NodeMatcher

from .icons import get_opticon


def setup_dropdown(app):
    app.add_node(dropdown_main, html=(visit_dropdown_main, depart_dropdown_main))
    app.add_node(dropdown_title, html=(visit_dropdown_title, depart_dropdown_title))
    app.add_directive("dropdown", DropdownDirective)
    app.add_post_transform(DropdownHtmlTransform)


class dropdown_main(nodes.Element, nodes.General):
    pass


class dropdown_title(nodes.TextElement, nodes.General):
    pass


def visit_dropdown_main(self, node):
    if node.get("opened"):
        self.body.append(self.starttag(node, "details", open="open"))
    else:
        self.body.append(self.starttag(node, "details"))


def depart_dropdown_main(self, node):
    self.body.append("</details>")


def visit_dropdown_title(self, node):
    self.body.append(self.starttag(node, "summary"))


def depart_dropdown_title(self, node):
    self.body.append("</summary>")


class DropdownDirective(SphinxDirective):
    optional_arguments = 1
    final_argument_whitespace = True
    has_content = True
    option_spec = {
        "container": directives.unchanged,
        "title": directives.unchanged,
        "body": directives.unchanged,
        "open": directives.flag,
        "name": directives.unchanged,
        "animate": lambda a: directives.choice(a, ("fade-in", "fade-in-slide-down")),
    }

    def run(self):

        # default classes
        classes = {
            "container_classes": ["mb-3"],
            "title_classes": [],
            "body_classes": [],
        }

        # add classes from options
        for element in ["container", "title", "body"]:
            if element not in self.options:
                continue
            value = self.options.get(element).strip()
            if value.startswith("+"):
                classes.setdefault(element + "_classes", []).extend(value[1:].split())
            else:
                classes[element + "_classes"] = value.split()

        # add animation classes
        if (
            "animate" in self.options
            and self.options["animate"] not in classes["container_classes"]
        ):
            classes["container_classes"].append(self.options["animate"])

        container = nodes.container(
            "",
            opened="open" in self.options,
            type="dropdown",
            has_title=len(self.arguments) > 0,
            **classes
        )
        if self.arguments:
            textnodes, messages = self.state.inline_text(self.arguments[0], self.lineno)
            container += nodes.paragraph(self.arguments[0], "", *textnodes)
            container += messages
        self.state.nested_parse(self.content, self.content_offset, container)
        self.add_name(container)
        return [container]


KEBAB = """\
<svg viewBox="0 0 36 24" width="36" height="16" xmlns="http://www.w3.org/2000/svg"
    class="octicon no-title" aria-hidden="true">
  <g xmlns="http://www.w3.org/2000/svg" class="jp-icon3">
    <circle cx="0" cy="12" r="6"></circle>
    <circle cx="18" cy="12" r="6"></circle>
    <circle cx="36" cy="12" r="6"></circle>
  </g>
</svg>"""


class DropdownHtmlTransform(SphinxPostTransform):
    default_priority = 200
    builders = ("html", "dirhtml", "singlehtml", "readthedocs")

    def run(self):
        matcher = NodeMatcher(nodes.container, type="dropdown")
        for node in self.document.traverse(matcher):

            open_marker = nodes.container(
                "",
                nodes.raw(
                    "", nodes.Text(get_opticon("chevron-up", size=24)), format="html"
                ),
                is_div=True,
                classes=["summary-up"],
            )
            closed_marker = nodes.container(
                "",
                nodes.raw(
                    "", nodes.Text(get_opticon("chevron-down", size=24)), format="html"
                ),
                is_div=True,
                classes=["summary-down"],
            )

            newnode = dropdown_main(
                opened=node["opened"],
                classes=["sphinx-bs", "dropdown", "card"] + node["container_classes"],
            )

            if node["has_title"]:
                title_children = node[0]
                body_children = node[1:]
            else:
                title_children = [
                    nodes.raw(
                        "...",
                        nodes.Text(
                            KEBAB
                            # Note the custom opticon here has thicker dots
                            # get_opticon("kebab-horizontal", classes="no-title",
                            # size=24)
                        ),
                        format="html",
                    )
                ]
                body_children = node

            newnode += dropdown_title(
                "",
                "",
                *title_children,
                closed_marker,
                open_marker,
                classes=["summary-title", "card-header"] + node["title_classes"]
            )
            body_node = nodes.container(
                "",
                *body_children,
                is_div=True,
                classes=["summary-content", "card-body"] + node["body_classes"]
            )
            for para in body_node.traverse(nodes.paragraph):
                para["classes"] = ([] if "classes" in para else para["classes"]) + [
                    "card-text"
                ]
            newnode += body_node
            # newnode += open_marker
            node.replace_self(newnode)
