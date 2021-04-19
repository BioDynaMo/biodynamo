set(classes BDMGlyphFilter BDMGlyph)

if(APPLE)
  vtk_module_add_module(BDM FORCE_STATIC CLASSES ${classes})
else()
  vtk_module_add_module(BDM CLASSES ${classes})
endif()

paraview_add_server_manager_xmls(XMLS  BDMGlyphFilter.xml)
