"""Directive dedicated to the vimeo platform."""

from . import utils


class vimeo(utils.video):
    """Empty video node class."""

    pass


class Vimeo(utils.Video):
    """Custom version of the Video Directive."""

    _node = vimeo
    _thumbnail_url = "https://vumbnail.com/{}.jpg"
    _platform = "vimeo"
    _platform_url = "https://player.vimeo.com/video/"
