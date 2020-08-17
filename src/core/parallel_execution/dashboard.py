import ipywidgets as widgets
from IPython.display import display, clear_output
import time, os, stat, sys
import random
import subprocess, pipes
from xml.etree import ElementTree as ET
from xml.dom import minidom

from subprocess import Popen, PIPE

class Entry:

  def __init__(self):
    self.name_ = ""
    self.params_ = []

  def AddParam(self, param):
    self.params_ = self.params_ + param

  def ToVBox(self):
    return VBox(self.params_)

class Dashboard:

  def __init__(self):
    self.hostfile_content_ = ""
    self.local_xml_filepath_ = ""
    self.remote_xml_filepath_ = ""
    self.remote_ = True
    self.style_ = {'description_width': '100px'}
    self.sim_name_ = "Binding Cells"
    self.tab_names_ = ['World', 'Simulation Objects', 'Biology Modules', 'Substances', 'Parallel Execution']
    self.tab_contents_ = {}

    # Create placeholders for each tab content (World and Parallel Execution)
    # are of different types
    for tab_name in self.tab_names_:
      if tab_name == "World":
        self.tab_contents_[tab_name] = widgets.GridspecLayout(4, 2)
      elif tab_name == "Parallel Execution":
        self.tab_contents_[tab_name] = widgets.VBox()
      else:
        self.tab_contents_[tab_name] = widgets.Accordion()

    ############################################################################
    ## World Tab
    ############################################################################
    self.tab_contents_['World'][0, 0] = widgets.Text(description='Name', value=self.sim_name_)
    self.tab_contents_['World'][1, 0] = widgets.IntText(description='Timesteps', value=4000)
    self.tab_contents_['World'][2, 0] = widgets.IntText(description='Min. Space', value=0)
    self.tab_contents_['World'][3, 0] = widgets.IntText(description='Max. Space', value=200)
    self.tab_contents_['World'][0, 1] = widgets.Checkbox(value=False, description='Visualization')
    self.tab_contents_['World'][1, 1] = widgets.IntText(description="Vis. Freq.", value=100)
    self.tab_contents_['World'][3, 1] = widgets.Text(description='Binary path', placeholder='/path/to/binary')

    ############################################################################
    ## Parallel Execution Tab
    ############################################################################
    hostfile_box = widgets.Textarea(placeholder="e.g. cloud.instance.01\ne.g. localhost",
                                description="hostfile", style=self.style_)
    self.nb_cores_ = widgets.IntText(description="Cores", value=2, style=self.style_)
    self.omp_threads_ = widgets.IntText(description="Threads / Core", value=1, style=self.style_)
    
    self.tab_contents_['Parallel Execution'] = widgets.VBox([hostfile_box, self.nb_cores_, self.omp_threads_])

  def AddEntry(self, tab_name, name, obj):
    idx = len(self.tab_contents_[tab_name].children)
    self.tab_contents_[tab_name].children = tuple(list(self.tab_contents_[tab_name].children) + [obj])
    self.tab_contents_[tab_name].set_title(idx, name)

  def AddSimulationObject(self, name, obj):
    tab_name = 'Simulation Objects'
    idx = len(self.tab_contents_[tab_name].children)
    self.tab_contents_[tab_name].children = tuple(list(self.tab_contents_[tab_name].children) + [obj])
    self.tab_contents_[tab_name].set_title(idx, name)

  def AddBiologyModule(self, name, obj):
    tab_name = 'Biology Modules'
    idx = len(self.tab_contents_[tab_name].children)
    self.tab_contents_[tab_name].children = tuple(list(self.tab_contents_[tab_name].children) + [obj])
    self.tab_contents_[tab_name].set_title(idx, name)
  
  def AddSubstance(self, name, obj):
    tab_name = 'Substances'
    idx = len(self.tab_contents_[tab_name].children)
    self.tab_contents_[tab_name].children = tuple(list(self.tab_contents_[tab_name].children) + [obj])
    self.tab_contents_[tab_name].set_title(idx, name)

  # Check if path exists on specified remote hostname
  def exists_remote(self, host, path):
      """Test if a file exists at path on a host accessible with SSH."""
      status = subprocess.call(
          ['ssh', host, 'test -f {}'.format(pipes.quote(path))])
      if status == 0:
          return True
      if status == 1:
          return False
      raise Exception('SSH failed')

  # Generate XML file from user input
  def generate_xml(self, tabs, sim_name):
      # Check if widget is of a type that we support for the dashboard
      def is_parseable_type(widget):
          allowed = [widgets.IntSlider, widgets.IntRangeSlider, widgets.FloatSlider, widgets.FloatLogSlider,
                    widgets.FloatRangeSlider, widgets.IntText, widgets.FloatText, widgets.HBox, widgets.VBox]
          if (isinstance(widget, tuple(allowed))):
              return True
          return False

      # Check if widget is a Slider (i.e. a range type)
      def is_range(widget):
          return "Slider" in str(type(widget)).split(".")[-1]

      # Check if widget is a LogSlider
      def is_log(widget):
        return "LogSlider" in str(type(widget)).split(".")[-1]

      # Set the value type of the XML node to the corresponding widget type
      # And fill in the data for each value type
      def set_value(elem, w):
          if (is_range(w)):
            if (is_log(w)):
              elem.set('value_type', 'log_range')
              base = ET.SubElement(elem, 'base')
              base.text = str(w.base)
              minn = ET.SubElement(elem, 'min')
              minn.text = str(w.min)
              maxx = ET.SubElement(elem, 'max')
              maxx.text = str(w.max)
              stride = ET.SubElement(elem, 'stride')
              stride.text = str(w.step)
            else:
              elem.set('value_type', 'range')
              minn = ET.SubElement(elem, 'min')
              minn.text = str(w.value[0])
              maxx = ET.SubElement(elem, 'max')
              maxx.text = str(w.value[1])
              stride = ET.SubElement(elem, 'stride')
              stride.text = str(w.step)
          else:
              elem.set('value_type', 'scalar')
              elem.text = str(w.value)

      # Traverse the widgets that are in the dashboard (i.e. which is called an
      # "accordion" in ipywidgets)
      def traverse_accordion(parent, n, accordion):
          j = 0
          for vbox in accordion.children:
              obj = ET.SubElement(parent, n)
              name = ET.SubElement(obj, 'name')
              name.text = accordion._titles["{}".format(j)]
              for w in vbox.children:
                  if (is_parseable_type(w)):
                      if isinstance(w, widgets.HBox) or isinstance(w, widgets.VBox):
                          param_name = w.children[0].value
                          w = w.children[1]
                      else:
                          param_name = w.description
                      temp = ET.SubElement(
                          obj, param_name.lower().replace(" ", "_"))
                      set_value(temp, w)
              j = j + 1

      # Traverse the World parameters
      def traverse_world_params(parent, vbox):
          for w in vbox.children:
            if (is_parseable_type(w)):
              if isinstance(w, widgets.HBox) or isinstance(w, widgets.VBox):
                  param_name = w.children[0].value
                  w = w.children[1]
              else:
                  param_name = w.description
              temp = ET.SubElement(
                  parent, param_name.lower().replace(" ", "_").replace(".", ""))
              set_value(temp, w)

      # For debugging purposes
      def prettify(elem):
          """Return a pretty-printed XML string for the Element.
          """
          rough_string = ET.tostring(elem, 'utf-8')
          reparsed = minidom.parseString(rough_string)
          return reparsed.toprettyxml(indent="  ")

      model = ET.Element('model')
      world = ET.SubElement(model, 'world')
      sim_objects = ET.SubElement(model, 'simulation_objects')
      bio_modules = ET.SubElement(model, 'biology_modules')
      substances = ET.SubElement(model, 'substances')

      for i in range(len(tabs.children)):
          tab_name = tabs._titles["{}".format(i)]
          accordion = tabs.children[i]
          if (tab_name == 'World'):
              traverse_world_params(world, accordion)
          elif (tab_name == 'Simulation Objects'):
              traverse_accordion(sim_objects, 'object', accordion)
          elif (tab_name == 'Biology Modules'):
              traverse_accordion(bio_modules, 'module', accordion)
          elif (tab_name == 'Substances'):
              traverse_accordion(substances, 'substance', accordion)
          i = 1 + 1

      myfile = open("{}.xml".format(sim_name), "w")
      myfile.write(prettify(model))
      self.local_xml_filepath_ = os.path.realpath(myfile.name)
      myfile.close()

  # Generate the hostfile for the MPI runtime
  def generate_hostfile(self):
    file = open("hostfile", "w")
    # remove spaces
    self.hostfile_content_.replace(" ", "")
    # remove empty lines
    self.hosts_ = list(filter(None, self.hostfile_content_.splitlines()))
    # if empty hostfile text area
    if not self.hostfile_content_:
      self.remote_ = False
      self.hostfile_content_ = "localhost\n"
    # if only "localhost" was given
    elif (len(self.hosts_) == 1) and (self.hosts_[0] == "localhost"):
      self.remote_ = False

    file.write(self.hostfile_content_)
    file.close()

  # Entry point for visualizing the dashboard
  def display(self):
      ############################################################################
      ## Compose Dashboard
      ############################################################################
      children = []
      for i in range(len(self.tab_names_)):
          temp = self.tab_contents_[self.tab_names_[i]]
          children.append(temp)
      tab = widgets.Tab()
      tab.children = children
      for i in range(len(self.tab_names_)):
          tab.set_title(i, str(self.tab_names_[i]))

      ############################################################################
      ## Create the Simulate button
      ############################################################################
      sim_button = widgets.Button(
          description="Start Simulation",
          layout=widgets.Layout(width='200px', height='50px'),
          button_style='info')
      sim_button.style.font_weight = 'bold'

      # extract simulation name and create XML file name
      sim_name = self.sim_name_.lower().replace(" ", "_")

      # Simulation button callback function
      def on_sim_click(b):
          with out:
              # generate the hostfile if specified in dashboard
              self.hostfile_content_ = self.tab_contents_["Parallel Execution"].children[0].value
              self.generate_hostfile()

              # Check if the binary exists at the given path
              self.binary_path_ = self.tab_contents_['World'][3, 1].value
              self.bin_dir_ = os.path.split(self.binary_path_)[0]
              self.bin_name_ = os.path.split(self.binary_path_)[1]
              self.dict_path_ = self.bin_dir_ + "/lib" + self.bin_name_ + "_dict.so"
              if not self.remote_:
                if not os.path.exists(self.binary_path_):
                  print("Error: The given binary path does not exist!")
                  return
              else:
                # For each remote machine check if path is valid
                for host in self.hosts_:
                  if not self.exists_remote(host, self.binary_path_):
                    print("Error: The given binary path does not exist on {}!".format(host))
                    return

              # generate xml file
              print("Generating XML parameter file...")
              self.generate_xml(tab, sim_name)
              # if we use remote machines, we need to copy the xml file to the
              # server(s), because the MPI command will expect it to be on the
              # remote machines
              if self.remote_:
                remote_build_dir = os.path.dirname(os.path.abspath(self.binary_path_))
                for host in self.hosts_:
                  full_host_path = host + ":" + remote_build_dir
                  p = Popen(["scp", self.local_xml_filepath_, full_host_path], stdout=PIPE, stderr=PIPE)
                  print(p.args)
                  pout = p.communicate()
                  print(pout[1].decode("utf-8"))
                self.remote_xml_filepath_ = remote_build_dir + "/" + self.local_xml_filepath_.split("/")[-1]

              # check if visualization was selected
              visualize = ""
              vis_freq = 0
              if self.tab_contents_['World'][0, 1].value:
                visualize = "--visualize"
                vis_freq = self.tab_contents_['World'][1, 1].value

              print("Executing BioDynaMo simulations in parallel...")
              filename = '{}.log'.format(sim_name)
              with open(filename, 'wb') as f:
                p = Popen(["mpirun",
                          "-hostfile", "hostfile",
                          "-x", "OMP_NUM_THREADS={}".format(self.omp_threads_.value),
                          "--use-hwthread-cpus",
                          "--bind-to", "hwthread",
                          "-np", str(self.nb_cores_.value),
                          self.binary_path_,
                          "{}".format(visualize),
                          "--xml={}".format(self.remote_xml_filepath_ if self.remote_ else self.local_xml_filepath_),
                          ], stdout=PIPE, stderr=PIPE)
                for line in iter(p.stdout.readline, b''):
                  sys.stdout.write(line)
                  f.write(line)

              pout = p.communicate()
              # print(p.args)
              # # print stdout
              print(pout[0].decode("utf-8"))
              # print stderr
              # print(pout[1].decode("utf-8"))
      
      sim_button.on_click(on_sim_click)

      ############################################################################
      ## Create the Clear Output button
      ############################################################################
      clear_button = widgets.Button(
          description="Clear Output",
          layout=widgets.Layout(width='200px', height='50px'),
          button_style='primary')
      clear_button.style.font_weight = 'bold'

      # Clear Output button callback function
      out = widgets.Output()
      def on_clear_click(b):
        with out:
          clear_output()

      clear_button.on_click(on_clear_click)

      # Display dashboard
      display(tab, widgets.HBox([sim_button, clear_button]), out)
