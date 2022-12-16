#!/usr/bin/env python
# -*- coding: utf-8 -*-
from . import utils


class youtube(utils.video):
    pass


class YouTube(utils.Video):
    _node = youtube
    _thumbnail_url = "https://i3.ytimg.com/vi/{}/maxresdefault.jpg"


def visit_youtube_node(self, node):
    return utils.visit_video_node(self, node,
                                  platform_url="https://www.youtube.com/embed/",
                                  platform_url_privacy="https://www.youtube-nocookie.com/embed/"
                                  )


def visit_youtube_node_latex(self, node):
    return utils.visit_video_node_latex(self, node, platform="youtube", platform_url="https://youtu.be/")


def unsupported_visit_youtube(self, node):
    return utils.unsupported_visit_video(self, node, platform="youtube")


_NODE_VISITORS = {
    'html': (visit_youtube_node, utils.depart_video_node),
    'latex': (visit_youtube_node_latex, utils.depart_video_node),
    'man': (unsupported_visit_youtube, None),
    'texinfo': (unsupported_visit_youtube, None),
    'text': (unsupported_visit_youtube, None)
}
