function [ loss ] = logPathLoss( txPower,refDist,refLoss, exp, distance )
%LOGPATHLOSS Summary of this function goes here
%   Detailed explanation goes here
    if (distance < refDist)
        loss = txPower - refLoss;
    else
        pathLoss = 10*exp*log10(distance/refDist);
        loss = -refLoss - pathLoss;
    end

end

