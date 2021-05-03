void P010_RootEveAdaptor()
{
  gPluginMgr->AddHandler("VisualizationAdaptor", "rooteve",
                        "bdm::RootEveAdaptor", "RootEveAdaptor",
                        "RootEveAdaptor()");
}
