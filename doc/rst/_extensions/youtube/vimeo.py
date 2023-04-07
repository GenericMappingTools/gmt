#!/usr/bin/env python
# -*- coding: utf-8 -*-
from . import utils


class vimeo(utils.video):
    pass


class Vimeo(utils.Video):
    _node = vimeo
    _thumbnail_url = "https://vumbnail.com/{}.jpg"


def visit_vimeo_node(self, node):
    return utils.visit_video_node(self, node, platform_url="https://player.vimeo.com/video/")


def visit_vimeo_node_latex(self, node):
    return utils.visit_video_node_latex(self, node, platform="vimeo", platform_url="https://player.vimeo.com/video/")


def unsupported_visit_vimeo(self, node):
    return utils.unsupported_visit_video(self, node, platform="vimeo")


_NODE_VISITORS = {
    'html': (visit_vimeo_node, utils.depart_video_node),
    'latex': (visit_vimeo_node_latex, utils.depart_video_node),
    'man': (unsupported_visit_vimeo, None),
    'texinfo': (unsupported_visit_vimeo, None),
    'text': (unsupported_visit_vimeo, None)
}
