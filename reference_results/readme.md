
# create
scenario-fmu-package --out ./reference_results/scenario.fmu -s "Alt;L;0,0;300,10000
Mach;L;0,0;300,1.2
heat_load;ZOH;0,50;150,4000
Wgt_On_Whl;ZOH;0,1;150,0
Aircraft_state;ZOH;0,0;150,4"

# run
scenario-fmu-run --fmu ./models/scenario.fmu --stop 1200
