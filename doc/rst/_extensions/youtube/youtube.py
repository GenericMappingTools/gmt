"""Directive dedicated to the youtube platform."""

from . import utils


class youtube(utils.video):
    """Empty video node class."""

    pass


class YouTube(utils.Video):
    """Custom version of the Video Directive."""

    _node = youtube
    _thumbnail_url = "https://i3.ytimg.com/vi/{}/maxresdefault.jpg"
    _platform = "youtube"
    _platform_url = "https://youtu.be/"
    _platform_url_privacy = "https://www.youtube-nocookie.com/embed/"


def visit_youtube_node(self, node):
    """Custom html visit node."""
    node["platform_url"] = "https://www.youtube.com/embed/"
    return utils.visit_video_node_html(self, node)


_NODE_VISITORS = utils._NODE_VISITORS.copy()
_NODE_VISITORS.update(html=(visit_youtube_node, utils.depart_video_node))
