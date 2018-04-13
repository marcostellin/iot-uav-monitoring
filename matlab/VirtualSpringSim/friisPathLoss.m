function [ loss ] = friisPathLoss( txPower,distance, freq )
%FRIISPATHLOSS Summary of this function goes here
%   Detailed explanation goes here
    
    denominator = 16*pi*pi*distance*distance;
    lambda = 299792458/freq;
    numerator = lambda*lambda;
    lossDB = -10*log10(numerator/denominator);
    loss = txPower - lossDB;
end

