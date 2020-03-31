set(classes BDMGlyphFilter BDMGlyph)

vtk_module_add_module(BDM CLASSES ${classes})

paraview_add_server_manager_xmls(XMLS  BDMGlyphFilter.xml)
