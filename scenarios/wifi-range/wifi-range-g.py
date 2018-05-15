from subprocess import call

dists = [100, 150, 200, 250, 300, 350, 400, 450, 500, 550, 600, 650, 700]
#dataModes = ["DsssRate1Mbps", "DsssRate2Mbps", "DsssRate5_5Mbps", "DsssRate11Mbps", "ErpOfdmRate6Mbps", "ErpOfdmRate9Mbps", "ErpOfdmRate12Mbps", "ErpOfdmRate18Mbps", "ErpOfdmRate24Mbps", "ErpOfdmRate36Mbps", "ErpOfdmRate48Mbps", "ErpOfdmRate54Mbps"]
dataModes = ["ErpOfdmRate6Mbps","ErpOfdmRate9Mbps", "ErpOfdmRate12Mbps", "OfdmRate6Mbps"]

for dataMode in dataModes:
  for dist in dists:
    command = "../../waf"
    arg = "--DataMode=" + dataMode + " " + "--Distance=" + str(dist)	
    tot = "\"wifi-range " + arg + "\""
    ns3call = call(command + " --run " + tot, shell="True")
