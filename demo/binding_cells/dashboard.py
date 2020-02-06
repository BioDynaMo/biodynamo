import ipywidgets as widgets
from IPython.display import display, clear_output
import time, os, stat, sys
import random
import subprocess, pipes
from xml.etree import ElementTree as ET
from xml.dom import minidom

from subprocess import Popen, PIPE

class Dashboard:

  def __init__(self):
    self.hostfile_content_ = ""
    self.local_xml_filepath = ""
    self.remote_xml_filepath = ""
    self.remote_ = True

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
      def is_parseable_type(widget):
          allowed = [widgets.IntSlider, widgets.IntRangeSlider, widgets.FloatSlider, widgets.FloatLogSlider,
                    widgets.FloatRangeSlider, widgets.IntText, widgets.FloatText, widgets.HBox, widgets.VBox]
          if (isinstance(widget, tuple(allowed))):
              return True
          return False

      def is_range(widget):
          return "Slider" in str(type(widget)).split(".")[-1]

      def is_log(widget):
        return "LogSlider" in str(type(widget)).split(".")[-1]

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
      self.local_xml_filepath = os.path.realpath(myfile.name)
      myfile.close()

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
      style = {'description_width': '100px'}
      style2 = {'description_width': '200px'}

      sim_objects = ['Monocyte', 'T_Cell']
      bio_modules = ['ConnectWithinRadius', 'Inhibition', 'StokesVelocity']
      substances  = ['Antibody']

      tab_contents = {}

      sim_name_field = widgets.Text(description='Name', value="Binding Cells")

      ############################################################################
      ## World Tab
      ############################################################################
      world_content = widgets.GridspecLayout(4, 2)
      world_content[0, 0] = sim_name_field
      world_content[1, 0] = widgets.IntText(description='Timesteps', value=4000)
      world_content[2, 0] = widgets.IntText(description='Min. Space', value=0)
      world_content[3, 0] = widgets.IntText(description='Max. Space', value=200)
      world_content[0, 1] = widgets.Checkbox(value=False, description='Visualization')
      world_content[1, 1] = widgets.IntText(description="Vis. Freq.", value=100)
      # world_content[3, 1] = widgets.FileUpload(multiple=False)
      world_content[3, 1] = widgets.Text(description='Binary path', placeholder='/path/to/binary')
      tab_contents['World'] = world_content

      ############################################################################
      ## Simulation Objects Tab
      ############################################################################
      tab_contents['Simulation Objects'] = widgets.Accordion(children=[
          widgets.VBox([
              widgets.IntText(description='Population', value=7272),
              widgets.IntText(description='Type', value=0),
              widgets.FloatText(description='Mass Density', value=1.067),
              widgets.FloatText(description='Diameter', value=1.5),
              widgets.FloatText(description='Velocity', value=2)
          ]),
          widgets.VBox([
              widgets.IntText(description='Population', value=2727),
              widgets.IntText(description='Type', value=1),
              widgets.FloatText(description='Mass Density', value=1.077),
              widgets.FloatText(description='Diameter', value=0.9),
              widgets.FloatText(description='Velocity', value=5),
              widgets.FloatText(description='Init Activation Mean', value=3),
              widgets.FloatText(description='Init Activation Sigma', value=1)
          ])
      ])
      tab_contents['Simulation Objects'].set_title(0, sim_objects[0])
      tab_contents['Simulation Objects'].set_title(1, sim_objects[1])

      ############################################################################
      ## Biology Modules Tab
      ############################################################################
      tab_contents['Biology Modules'] = widgets.Accordion(children=[
          widgets.VBox([
              widgets.FloatText(description='Binding radius', value=5, style=style)
          ]),
          widgets.VBox([
              widgets.VBox([widgets.Label('Sigma'), widgets.FloatText(value=1)]),
              widgets.VBox([widgets.Label('Mu'), widgets.FloatText(value=-8.5)])
          ]),
          widgets.VBox([
              widgets.FloatText(description='Viscosity', value=0.089, style=style),
              widgets.FloatText(description='Mass Density Fluid', value=0.997, style=style)
          ])
      ])
      tab_contents['Biology Modules'].set_title(0, bio_modules[0])
      tab_contents['Biology Modules'].set_title(1, bio_modules[1])
      tab_contents['Biology Modules'].set_title(2, bio_modules[2])

      ############################################################################
      ## Substances Tab
      ############################################################################
      log_slider = widgets.HBox([widgets.Label('Amount'), 
                           widgets.FloatLogSlider(value=10, min=-15, max=-6, step=0.125, base=10)])
      log_min = widgets.FloatText(description='Min', value=-15, style=style)
      log_max = widgets.FloatText(description='Max', value=-6, style=style)
      log_step = widgets.FloatText(description='Step', value=0.125, style=style)

      widgets.link((log_slider.children[1], 'min'), (log_min, 'value'))
      widgets.link((log_slider.children[1], 'max'), (log_max, 'value'))
      widgets.link((log_slider.children[1], 'step'), (log_step, 'value'))
      tab_contents['Substances'] = widgets.Accordion(children=[
          widgets.VBox([
              log_slider, log_min, log_max, log_step,
              widgets.FloatText(description='Diffusion Rate', value=0.0, style=style),
              widgets.FloatText(description='Decay Rate', value=0.0),
              widgets.IntText(description='Resolution', value=10)
          ])
      ])
      tab_contents['Substances'].set_title(0, substances[0])

      ############################################################################
      ## Parallel Execution Tab
      ############################################################################
      hostfile_box = widgets.Textarea(placeholder="e.g. cloud.instance.01\ne.g. localhost",
                                  description="hostfile", style=style)
      nb_cores = widgets.IntText(description="Cores", value=2, style=style)
      omp_threads = widgets.IntText(description="Threads / Core", value=1, style=style)
      
      tab_contents['Parallel Execution'] = widgets.VBox([hostfile_box, nb_cores, omp_threads])

      ############################################################################
      ## Compose Dashboard
      ############################################################################
      tab_names = ['World'] + ['Simulation Objects'] + \
          ['Biology Modules'] + ['Substances'] + ['Parallel Execution']
      children = []
      for i in range(len(tab_names)):
          temp = tab_contents[tab_names[i]]
          children.append(temp)
      tab = widgets.Tab()
      tab.children = children
      for i in range(len(tab_names)):
          tab.set_title(i, str(tab_names[i]))

      ############################################################################
      ## Create the Simulate button
      ############################################################################
      sim_button = widgets.Button(
          description="Start Simulation",
          layout=widgets.Layout(width='200px', height='50px'),
          button_style='info')
      sim_button.style.font_weight = 'bold'

      # extract simulation name and create XML file name
      sim_name = sim_name_field.value.lower().replace(" ", "_")

      # Simulation button callback function
      def on_sim_click(b):
          with out:
              # generate the hostfile if specified in dashboard
              self.hostfile_content_ = hostfile_box.value
              self.generate_hostfile()

              # Check if the binary exists at the given path
              binary_path = world_content[3, 1].value
              # for debugging
              if not binary_path:
                binary_path = "/data/ahhesam/biodynamo/build/demo/binding_cells/build/binding_cells"
              if not self.remote_:
                if not os.path.exists(binary_path):
                  print("Error: The given binary path does not exist!")
                  return
              else:
                # For each remote machine check if path is valid
                for host in self.hosts_:
                  if not self.exists_remote(host, binary_path):
                    print("Error: The given binary path does not exist on {}!".format(host))
                    return

              # generate xml file
              print("Generating XML parameter file...")
              self.generate_xml(tab, sim_name)
              # if we use remote machines, we need to copy the xml file to the
              # server(s), because the MPI command will expect it to be on the
              # remote machines
              if self.remote_:
                self.remote_build_dir = os.path.dirname(os.path.abspath(binary_path))
                for host in self.hosts_:
                  full_host_path = host + ":" + self.remote_build_dir
                  p = Popen(["scp", self.local_xml_filepath, full_host_path], stdout=PIPE, stderr=PIPE)
                  print(p.args)
                  pout = p.communicate()
                  print(pout[1].decode("utf-8"))
                self.remote_xml_filepath = self.remote_build_dir + "/" + self.local_xml_filepath.split("/")[-1]

              # check if visualization was selected
              visualize = ""
              vis_freq = 0
              if world_content[0, 1].value:
                visualize = "--visualize"
                vis_freq = world_content[1, 1].value

              print("Executing BioDynaMo simulations in parallel...")
              filename = '{}.log'.format(sim_name)
              with open(filename, 'wb') as f:
                p = Popen(["mpirun",
                          "-hostfile", "hostfile",
                          "-x", "OMP_NUM_THREADS={}".format(omp_threads.value),
                          "--use-hwthread-cpus",
                          "--bind-to", "hwthread",
                          "-np", str(nb_cores.value),
                          binary_path,
                          "{}".format(visualize),
                          "--xml={}".format(self.remote_xml_filepath if self.remote_ else self.local_xml_filepath),
                          ], stdout=PIPE, stderr=PIPE)
                for line in iter(p.stdout.readline, b''):
                  sys.stdout.write(line)
                  f.write(line)

              pout = p.communicate()
              print(p.args)
              # # print stdout
              # print(pout[0].decode("utf-8"))
              # print stderr
              print(pout[1].decode("utf-8"))
      
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
