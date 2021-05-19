void P020_RootEveAdaptor()
{
  gPluginMgr->AddHandler("VisualizationAdaptor", "rooteve",
                        "bdm::RootEveAdaptor", "RootEveAdaptor",
                        "RootEveAdaptor()");
}
