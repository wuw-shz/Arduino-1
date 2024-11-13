import os
Import("env")

# Specify the Wokwi project ID (from Wokwi project URL)
wokwi_project_id = "414278960369754113"

# Define a custom command to run Wokwi CLI simulation
def run_wokwi_simulation(source, target, env):
    os.system(f"wokwi-cli exec {env['PROJECT_DIR']} --id {wokwi_project_id}")

env.AddPreAction("upload", run_wokwi_simulation)
