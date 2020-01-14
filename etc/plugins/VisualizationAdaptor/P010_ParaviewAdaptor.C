void P010_ParaviewAdaptor()
{
  gPluginMgr->AddHandler("VisualizationAdaptor", "paraview",
                        "bdm::ParaviewAdaptor", "VisualizationAdaptor",
                        "ParaviewAdaptor()");
}