#!/usr/bin/env python
# -*- coding: utf-8 -*-

import re
from docutils import nodes
from docutils.parsers.rst import directives, Directive
from sphinx.util import logging
from sphinx.util.console import brown
from sphinx.util import status_iterator
import os
import requests
from pathlib import Path

logger = logging.getLogger(__name__)

CONTROL_HEIGHT = 30

THUMBNAIL_DIR = "_video_thumbnail"


def get_size(d, key):
    if key not in d:
        return None
    m = re.match("(\d+)(|%|px)$", d[key])
    if not m:
        raise ValueError("invalid size %r" % d[key])
    return int(m.group(1)), m.group(2) or "px"


def css(d):
    return "; ".join(sorted("%s: %s" % kv for kv in d.items()))


class video(nodes.General, nodes.Element):
    pass


def visit_video_node(self, node, platform_url, platform_url_privacy=None):
    aspect = node["aspect"]
    width = node["width"]
    height = node["height"]
    url_parameters = node["url_parameters"]
    if node.get('privacy_mode') and platform_url_privacy:
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
            "src": "{}{}{}".format(platform_url,node['id'],url_parameters),
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
            "src":  "{}{}{}".format(platform_url,node['id'],url_parameters),
            "style": css(style),
        }
    if node["align"] != None: div_style["text-align"] = node["align"]
    attrs["allowfullscreen"] = "true"
    div_attrs = {
        "CLASS": "video_wrapper",
        "style": css(div_style),
    }
    if node["align"] != None: div_attrs["CLASS"] += " align-%s" % node["align"]
    self.body.append(self.starttag(node, "div", **div_attrs))
    self.body.append(self.starttag(node, "iframe", **attrs))
    self.body.append("</iframe></div>")


def depart_video_node(self, node):
    pass


def visit_video_node_latex(self, node, platform, platform_url):
    
    folder = r"\graphicspath{ {./%s/}{./} }" % THUMBNAIL_DIR
    if folder not in self.elements["preamble"]:
        self.elements["preamble"] += folder + "\n"

    macro = f"\\sphinxcontrib{platform}"
    if macro not in self.elements["preamble"]:
        cmd = r"\newcommand{%s}[3]{\begin{quote}\begin{center}\fbox{\url{#1#2#3}}\end{center}\end{quote}}" % macro
        self.elements["preamble"] += cmd + "\n"
    
    self.body.append('%s{%s}{%s}{%s}\n' % (macro, platform_url, node['id'], node['url_parameters']))


class Video(Directive):
    _node = None # Subclasses should replace with node class.
    _thumbnail_url = "{}" # url to retreive thumbnail images
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
        
        env = self.state.document.settings.env 
        video_id = self.arguments[0]
        url = self._thumbnail_url.format(video_id)
        env.remote_images[url] = Path(THUMBNAIL_DIR, f"{video_id}.jpg")
        env.images.add_file('', env.remote_images[url])
    
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
            
        width = get_size(self.options, "width")
        height = get_size(self.options, "height")
        url_parameters = self.options.get("url_parameters", "")
        return [self._node(id=self.arguments[0], aspect=aspect, width=width,
                height=height, align=align, url_parameters=url_parameters,
                privacy_mode=self.options.get('privacy_mode'))]


def unsupported_visit_video(self, node, platform):
    self.builder.warn(
        '{}: unsupported output format (node skipped)'.format(platform)
    )
    raise nodes.SkipNode

def download_images(app, env):

    iterator = app.builder.status_iterator if hasattr(app.builder, 'status_iterator') else status_iterator
    msg = 'Downloading remote images...'
    for src in iterator(env.remote_images, msg, brown, len(env.remote_images)):
        
        dst = Path(app.outdir) / env.remote_images[src]
        if not dst.is_file():
            logger.info(f'{src} -> {dst} (downloading)')
            with open(dst, 'wb') as f:
                try:
                    f.write(requests.get(src).content)
                except requests.ConnectionError:
                    logger.info(f'Cannot download "{src}"')
        else:
            logger.info(f'{src} -> {dst} (already in cache)')
            
def configure_image_download(app):
    
    app.env.remote_images = {}

    output_dir = Path(app.outdir) / THUMBNAIL_DIR
    output_dir.mkdir(exist_ok=True)
    app.config.html_static_path.append(str(output_dir))
