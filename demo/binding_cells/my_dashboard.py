from dashboard import *

d = Dashboard()

############################################################################
## My Simulation Objects
############################################################################

# Monocyte parameters
monocyte = widgets.VBox([
    widgets.IntText(description='Population', value=7272),
    widgets.IntText(description='Type', value=0),
    widgets.FloatText(description='Mass Density', value=1.067),
    widgets.FloatText(description='Diameter', value=1.5),
    widgets.FloatText(description='Velocity', value=2)
])

# T-Cell parameters
t_cell = widgets.VBox([
    widgets.IntText(description='Population', value=2727),
    widgets.IntText(description='Type', value=1),
    widgets.FloatText(description='Mass Density', value=1.077),
    widgets.FloatText(description='Diameter', value=0.9),
    widgets.FloatText(description='Velocity', value=5),
    widgets.FloatText(description='Init Activation Mean', value=3),
    widgets.FloatText(description='Init Activation Sigma', value=1)
])

d.AddSimulationObject('Monocyte', monocyte)
d.AddSimulationObject('T_Cell', t_cell)

############################################################################
## My Biology Modules
############################################################################

# ConnectWithRadius biology module parameters
connect_with_radius = widgets.VBox([
    widgets.FloatText(description='Binding Radius', value=5, style=d.style_)
])

# Inhibition biology module parameters
inhibition = widgets.VBox([
    widgets.VBox([widgets.Label('Sigma'), widgets.FloatText(value=1)]),
    widgets.VBox([widgets.Label('Mu'), widgets.FloatText(value=-8.5)])
])

# StokesVelocity biology module parameters
stokesvelocity = widgets.VBox([
    widgets.FloatText(description='Viscosity', value=0.089, style=d.style_),
    widgets.FloatText(description='Mass Density Fluid', value=0.997, style=d.style_)
])

d.AddBiologyModule('ConnectWithinRadius', connect_with_radius)
d.AddBiologyModule('Inhibition', inhibition)
d.AddBiologyModule('StokesVelocity', stokesvelocity)

############################################################################
## My Substances
############################################################################
log_slider = widgets.HBox([widgets.Label('Amount'), 
                      widgets.FloatLogSlider(value=10, min=-15, max=-6, step=0.125, base=10)])
log_min = widgets.FloatText(description='Min', value=-15, style=d.style_)
log_max = widgets.FloatText(description='Max', value=-6, style=d.style_)
log_step = widgets.FloatText(description='Step', value=0.125, style=d.style_)

widgets.link((log_slider.children[1], 'min'), (log_min, 'value'))
widgets.link((log_slider.children[1], 'max'), (log_max, 'value'))
widgets.link((log_slider.children[1], 'step'), (log_step, 'value'))

antibody = widgets.VBox([
    log_slider, log_min, log_max, log_step,
    widgets.FloatText(description='Diffusion Rate', value=0.0, style=d.style_),
    widgets.FloatText(description='Decay Rate', value=0.0),
    widgets.IntText(description='Resolution', value=10)
])

d.AddSubstance('Antibody', antibody)

d.display()
