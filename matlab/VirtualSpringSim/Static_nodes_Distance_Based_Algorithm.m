clear all;
close all;

N_UAVS = 3;     %Number of Aerial vehicles
N_EUS = 10;     %Number of ground nodes
K_ATA = 0.5;    %Stifness of springs in the AtA links
TIME = 1000;    %Total running time of the simulation
DELTA = 1;      %Time increment
SIZE = 100;     %Size of scenario
SCALING = 10;
RANGE_ATA = 300; %Range of transmission of AtA links
RANGE_ATG = 100; %Range of transmission of AtG links
velUavs = 10;    %Speed of UAVs
L0_ATA = 100;
L0_ATG = 50;

%% LINK BUDGET COMPUTATION PARAMS %%
TX_POWER_ATA = 16; %Power of WiFi links in dbm
TX_POWER_ATG = 14; %Power of LoRa nodes
WIFI_F = 2.4e9;
PATH_EXP = 2;      %Log distance Path loss exponent
REF_DIST = 1;      %Log distance reference distance
REF_LOSS = 46;     %Loss at reference distance

%% DISPLACEMENT COMPUTATION PARAMETERS
ALFA = 2;           %Propagation decay loss exponent
LB_REQ_ATA = 40;   %Required LB AtA
LB_REQ_ATG = 60;  %Required LB AtG     
LB_TH_ATA =  -75;   %Minimum acceptable receiver sensitivity (WiFi)
LB_TH_ATG =  -120;  %Minimum acceptable receiver sensitivity (LoRa)


%% SIMULATION

%Initialize UAVs current position vector
UAVsCurPos = zeros(N_UAVS, 2);
startAngle = 180;
increment = 180/N_UAVS;
radius = 100;               
for i=(1:N_UAVS)
    angle = increment*i;
    rads = angle*pi/180;
    tx = SIZE*SCALING/2 + radius*cos(rads);
    ty = 0 + radius*sin(rads);
    UAVsCurPos(i,:) = [tx,ty];
end

for j=(1:N_EUS)
    EUsCurPos(j,:) = [400,20*j];
end


%Initialize the AtA distances
AtADists = zeros(N_UAVS, N_UAVS);

%Initialize the AtA connectivity matrix
AtAConn = zeros(N_UAVS, N_UAVS);

for i=(1:DELTA:TIME)
    
    for j=(1:N_EUS)
        EUsCurPos(j,:) = EUsCurPos(j,:) +  [10,0];
    end
    
    %Compute the AtA distances 
    for j=(1:N_UAVS)
        for k=(1:N_UAVS)
            AtADists(j,k) = distance(UAVsCurPos(j,:), UAVsCurPos(k,:));
        end
    end
    
    %Compute the AtA connectivity matrix
    for j=(1:N_UAVS)
        for k=(1:N_UAVS)
            if j ~= k && AtADists(j,k) < RANGE_ATA
                AtAConn(j,k) = 1;
            else
                AtAConn(j,k) = 0;
            end
        end
    end
    
    %Initialize the AtG node counter
    AtGCounter = zeros(N_UAVS,1);
    
    %Fill the AtG counter
    for j = (1:N_UAVS)
        for k = (1:N_EUS)
            distance(UAVsCurPos(j,:), EUsCurPos(k,:))
            if distance(UAVsCurPos(j,:), EUsCurPos(k,:)) < RANGE_ATG
                
                AtGCounter(j) = AtGCounter(j)+1;
            end
        end
    end
    
    %Initialize the k_ATG vector
    kAtG = zeros(N_UAVS,1);
    
    %Compute k_ATG
    for j = (1:N_UAVS)
        max_ni = 1;
        for k = (1:N_UAVS)
            if AtAConn(j,k) == 1 && AtGCounter(k) > max_ni
                max_ni = AtGCounter(k);
            end
        end
        kAtG(j) = AtGCounter(j)/max_ni;
    end
    
    %Initialize the total forces vector
    forces = zeros(N_UAVS,2);
    
    %Compute the AtA forces
    for j = (1:N_UAVS)
        for k = (1:N_UAVS)
            if AtAConn(j,k) == 1 && AtADists(j,k) > RANGE_ATA-100
                disp = AtADists(j,k)-L0_ATA; 
                d_x = -UAVsCurPos(j,1) + UAVsCurPos(k,1);
                d_y = -UAVsCurPos(j,2) + UAVsCurPos(k,2);
                force = K_ATA*disp;
                h = force/sqrt(d_x^2 + d_y^2);
                forces(j,:) = forces(j,:) + [h*d_x, h * d_y];
            end
        end
    end
     
    %Compute the AtG forces
    for j = (1:N_UAVS)
        for k = (1:N_EUS)
            d = distance(UAVsCurPos(j,:), EUsCurPos(k,:));
            if d < RANGE_ATG
                disp = d -L0_ATG; 
                
                d_x = -UAVsCurPos(j,1) + EUsCurPos(k,1);
                d_y = -UAVsCurPos(j,2) + EUsCurPos(k,2);
                force = kAtG(j)*disp;
                h = force/sqrt(d_x^2 + d_y^2);
                forces(j,:) = forces(j,:) + [h*d_x, h * d_y];
            end
        end
    end
    
    UAVsNextPos = UAVsCurPos;
    %Compute new positions of UAVs
    for j=(1:N_UAVS)
        if (forces(j,1) ~= 0 && forces(j,2) ~= 0)
            d_x = forces(j,1);
            d_y = forces(j,2);

            h = velUavs/sqrt(d_x^2 + d_y^2);

            newx = UAVsCurPos(j,1) + h*d_x*DELTA;
            newy = UAVsCurPos(j,2) + h*d_y*DELTA;

            UAVsNextPos(j,:) = [newx,newy];
        end
    end
    UAVsCurPos = UAVsNextPos;
    
    plot(UAVsCurPos(:,1)./10,UAVsCurPos(:,2)./10, 'b*');
    hold on;
    plot(50, 0, 'c*');
    plot(EUsCurPos(:,1)/10, EUsCurPos(:,2)/10, 'g*');
    viscircles([UAVsCurPos(:,1)./SCALING, UAVsCurPos(:,2)./SCALING],RANGE_ATA/SCALING*ones(length(UAVsCurPos(:,1)),1));
    viscircles([UAVsCurPos(:,1)./SCALING, UAVsCurPos(:,2)./SCALING],RANGE_ATG/SCALING*ones(length(UAVsCurPos(:,1)),1));
    hold off;
    axis([-SIZE SIZE -SIZE SIZE]);
    grid on;
    pause(.5)
    
end



plot(UAVsCurPos(:,1)./10, UAVsCurPos(:,2)./10,'b*');
axis([0 SIZE 0 SIZE]);
grid on;
