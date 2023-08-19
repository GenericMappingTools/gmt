"""Skeleton of the video directive ready to be extended for specific providers."""

import re
from pathlib import Path

import requests
from docutils import nodes
from docutils.parsers.rst import Directive, directives
from sphinx.util import logging, status_iterator
from sphinx.util.console import brown

logger = logging.getLogger(__name__)

CONTROL_HEIGHT = 30

THUMBNAIL_DIR = "_video_thumbnail"

# -- helper methods ------------------------------------------------------------


def get_size(d, key):
    """Return a valid css size and unit."""
    if key not in d:
        return None
    m = re.match(r"(\d+)(|%|px)$", d[key])
    if not m:
        raise ValueError("invalid size %r" % d[key])
    return int(m.group(1)), m.group(2) or "px"


def css(d):
    """Return a valid css style string."""
    return "; ".join(sorted("%s: %s" % kv for kv in d.items()))


# -- node and directive definition ---------------------------------------------


class video(nodes.General, nodes.Element):
    """Video node."""

    pass


class Video(Directive):
    """Abstract Video directive."""

    _node = None
    "Subclasses should replace with node class."

    _thumbnail_url = "{}"
    "url to retrieve thumbnail images"

    _platform = ""
    "name of the platform"

    _platform_url = ""
    "url of the platform video provider"

    _platform_url_privacy = ""
    "the aleternative url to provide the video privately"

    has_content = True
    required_arguments = 1
    optional_arguments = 0
    final_argument_whitespace = False
    option_spec = {
        "width": directives.unchanged,
        "height": directives.unchanged,
        "aspect": directives.unchanged,
        "align": directives.unchanged,
        "url_parameters": directives.unchanged,
        "privacy_mode": directives.unchanged,
    }

    def run(self):
        """Run the directive."""
        env = self.state.document.settings.env
        video_id = self.arguments[0]
        url = self._thumbnail_url.format(video_id)
        env.video_remote_images[url] = Path(THUMBNAIL_DIR, f"{video_id}.jpg")
        env.images.add_file("", env.video_remote_images[url])

        if "aspect" in self.options:
            aspect = self.options.get("aspect")
            m = re.match(r"(\d+):(\d+)", aspect)
            if m is None:
                raise ValueError("invalid aspect ratio %r" % aspect)
            aspect = tuple(int(x) for x in m.groups())
        else:
            aspect = None

        alignment = ["left", "center", "right"]
        if "align" in self.options:
            align = self.options.get("align")
            if align not in alignment:
                raise ValueError(f"invalid alignment choices are: {alignment}")
        else:
            align = None

        return [
            self._node(
                id=self.arguments[0],
                aspect=aspect,
                width=get_size(self.options, "width"),
                height=get_size(self.options, "height"),
                align=align,
                url_parameters=self.options.get("url_parameters", ""),
                privacy_mode=self.options.get("privacy_mode"),
                platform=self._platform,
                platform_url=self._platform_url,
                platform_url_privacy=self._platform_url_privacy,
            )
        ]


# -- builder specific methods --------------------------------------------------


def visit_video_node_html(self, node, platform_url_privacy=None):
    """Visit html video node."""
    aspect = node["aspect"]
    width = node["width"]
    height = node["height"]
    url_parameters = node["url_parameters"]
    platform_url = node["platform_url"]
    platform_url_privacy = node["platform_url_privacy"]
    if node.get("privacy_mode") and platform_url_privacy:
        platform_url = platform_url_privacy

    if aspect is None:
        aspect = 16, 9

    div_style = {}
    if (height is None) and (width is not None) and (width[1] == "%"):
        div_style = {
            "padding-top": "%dpx" % CONTROL_HEIGHT,
            "padding-bottom": "%f%%" % (width[0] * aspect[1] / aspect[0]),
            "width": "%d%s" % width,
            "position": "relative",
        }
        style = {
            "position": "absolute",
            "top": "0",
            "left": "0",
            "width": "100%",
            "height": "100%",
            "border": "0",
        }
        attrs = {
            "src": "{}{}{}".format(platform_url, node["id"], url_parameters),
            "style": css(style),
        }
    else:
        if width is None:
            if height is None:
                width = 560, "px"
            else:
                width = height[0] * aspect[0] / aspect[1], "px"
        if height is None:
            height = width[0] * aspect[1] / aspect[0], "px"
        style = {
            "width": "%d%s" % width,
            "height": "%d%s" % (height[0] + CONTROL_HEIGHT, height[1]),
            "border": "0",
        }
        attrs = {
            "src": "{}{}{}".format(platform_url, node["id"], url_parameters),
            "style": css(style),
        }
    if node["align"] is not None:
        div_style["text-align"] = node["align"]
    attrs["allowfullscreen"] = "true"
    div_attrs = {
        "CLASS": "video_wrapper",
        "style": css(div_style),
    }
    if node["align"] is not None:
        div_attrs["CLASS"] += " align-%s" % node["align"]
    self.body.append(self.starttag(node, "div", **div_attrs))
    self.body.append(self.starttag(node, "iframe", **attrs))
    self.body.append("</iframe></div>")


def visit_video_node_epub(self, node):
    """Visit epub video node."""
    url_parameters = node["url_parameters"]
    link_url = "{}{}{}".format(node["platform_url"], node["id"], url_parameters)

    self.body.append(self.starttag(node, "a", CLASS="video_link_url", href=link_url))
    self.body.append(link_url)
    self.body.append("</a>")


def visit_video_node_latex(self, node):
    """Visit latex video node."""
    folder = r"\graphicspath{ {./%s/}{./} }" % THUMBNAIL_DIR
    if folder not in self.elements["preamble"]:
        self.elements["preamble"] += folder + "\n"

    macro = f"\\sphinxcontrib{node['platform']}"
    if macro not in self.elements["preamble"]:
        cmd = (
            r"\newcommand{%s}[3]{\begin{quote}\begin{center}\fbox{\url{#1#2#3}}\end{center}\end{quote}}"
            % macro
        )
        self.elements["preamble"] += cmd + "\n"

    self.body.append(
        "{}{{{}}}{{{}}}{{{}}}\n".format(
            macro, node["platform_url"], node["id"], node["url_parameters"]
        )
    )


def visit_video_node_unsupported(self, node):
    """Visit unsuported video node."""
    logger.warning(f"{node['platform']}: unsupported output format (node skipped)")
    raise nodes.SkipNode


def depart_video_node(self, node):
    """Depart any video node."""
    pass


_NODE_VISITORS = {
    "html": (visit_video_node_html, depart_video_node),
    "epub": (visit_video_node_epub, depart_video_node),
    "latex": (visit_video_node_latex, depart_video_node),
    "man": (visit_video_node_unsupported, depart_video_node),
    "texinfo": (visit_video_node_unsupported, depart_video_node),
    "text": (visit_video_node_unsupported, depart_video_node),
}

# -- manage dowloaded images ---------------------------------------------------


def merge_download_images(app, env, docnames, other):
    """Merge remote images, when using parallel processing."""
    env.video_remote_images.update(other.video_remote_images)


def download_images(app, env):
    """Download thumbnails for the latex build."""
    # images should only be downloaded if the builder is Latex related
    if "latex" not in app.builder.name:
        return

    iterator = (
        app.builder.status_iterator
        if hasattr(app.builder, "status_iterator")
        else status_iterator
    )
    msg = "Downloading remote images..."
    nb_images = len(env.video_remote_images)
    for src in iterator(env.video_remote_images, msg, brown, nb_images):

        dst = Path(app.outdir) / env.video_remote_images[src]
        if not dst.is_file():
            logger.info(f"{src} -> {dst} (downloading)")
            with open(dst, "wb") as f:
                try:
                    f.write(requests.get(src).content)
                except requests.ConnectionError:
                    logger.info(f'Cannot download "{src}"')
        else:
            logger.info(f"{src} -> {dst} (already in cache)")


def configure_image_download(app):
    """Configure Sphinx to download video thumbnails."""
    app.env.video_remote_images = {}

    output_dir = Path(app.outdir) / THUMBNAIL_DIR
    output_dir.mkdir(exist_ok=True)
    app.config.html_static_path.append(str(output_dir))
