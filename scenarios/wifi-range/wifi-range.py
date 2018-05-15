from subprocess import call

dists = [100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700, 750, 800, 850, 900, 950, 1000]
dataModes = ["VhtMcs0", "VhtMcs1", "VhtMcs2", "VhtMcs3", "VhtMcs4", "VhtMcs5", "VhtMcs6", "VhtMcs7", "VhtMcs8", "VhtMcs9"]


for dataMode in dataModes:
  for dist in dists:
    command = "../../waf"
    arg = "--DataMode=" + dataMode + " " + "--Distance=" + str(dist)	
    tot = "\"wifi-range " + arg + "\""
    ns3call = call(command + " --run " + tot, shell="True")
	
