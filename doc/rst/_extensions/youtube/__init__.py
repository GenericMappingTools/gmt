from  . import youtube, vimeo, utils
    
def setup(app):
    app.add_node(youtube.youtube, **youtube._NODE_VISITORS)
    app.add_directive("youtube", youtube.YouTube)
    app.add_node(vimeo.vimeo, **vimeo._NODE_VISITORS)
    app.add_directive("vimeo", vimeo.Vimeo)
    app.connect('builder-inited', utils.configure_image_download)
    app.connect('env-updated', utils.download_images)
