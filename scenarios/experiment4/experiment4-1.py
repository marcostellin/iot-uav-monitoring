from subprocess import call

sf1 = 5
sf2 = 5
period = [10,20,30,60,100]
nNodes = [150,300]

for num in nNodes:
	for t in period:
		command = "../../waf"
		arg = "--DataRate1=" + str(sf1) + " " + "--DataRate2=" + str(sf2) + " " + "--Interval=" + str(t) + " " + "--nNodes=" + str(num)
		tot = "\"experiment4 " + arg + "\""
		ns3call = call(command + " --run " + tot, shell="True")
