load ('TxRx1.mat','TxRx1')
load ('TxRxpairs.mat','TxRxpairs')
Nodes=TxRxpairs; clear TxRxpairs
TxRx = TxRx1;
gridXres = 0.04;
gridYres = gridXres;
c = 3e8;
M = 50;
freq = 2.4e9;
lambda = c/freq;
k0 = 2*pi/lambda;
RoomLength = 2;
RoomWidth = RoomLength;
tx = [gridXres/2:gridXres:RoomLength-gridXres/2]-RoomLength/2;
ty = [RoomWidth-gridYres/2:-gridYres:gridYres/2]-RoomWidth/2;
[x,y] = meshgrid(tx,ty);
cellrad = (sqrt(gridXres^2/pi)*2)/2;
Gridarea = (4*pi*cellrad/(2*k0))*besselj(1,k0*cellrad);

[xt_d, xr_d] = meshgrid(TxRx(:,1), TxRx(:,1));
[yt_d, yr_d] = meshgrid(TxRx(:,2), TxRx(:,2));
distTxRx = sqrt((xt_d-xr_d).^2 + (yt_d-yr_d).^2);
E_d =   (1i/4)*besselh(0,1,k0*distTxRx);

clear xt_d xp_d yt_d yp_d G_Rx_Tx G_Tx_Rx Theta_Tx_Rx Theta_Rx_Tx

[xt, xp] = meshgrid(TxRx(:,1), x(:));
[yt, yp] = meshgrid(TxRx(:,2), y(:));
distTxRn = sqrt((xt-xp).^2 + (yt-yp).^2);
E_inc =   (1i/4)*besselh(0,1,k0*distTxRn);

clear xt yt xp yp Theta_Tx_Rn Ref_theta_Tx_Rn G_Tx_Rn

Fryt = zeros(length(Nodes(1,:)), M^2);
Fryt_k = zeros(length(Nodes(1,:)), M^2);
[xr, xpr] = meshgrid(TxRx(:,1), x(:));
[yr, ypr] = meshgrid(TxRx(:,2), y(:));
distRxRn = (sqrt((xr-xpr).^2 + (yr-ypr).^2))';
Zryt=((1i*pi*cellrad/(2*k0))*...
    besselj(1,k0*cellrad)*...
    besselh(0,1,k0*distRxRn));
for i = 1:length(Nodes(1,:))
    Fryt(i,:) = ((k0^2)*((Zryt(Nodes(2,i),:).*...
        (E_inc(:,Nodes(1,i))).')./...
        (E_d(Nodes(2,i), Nodes(1,i)))));
end

FrytB =  [real(Fryt) -imag(Fryt)];
if isfile('kernel.mat')
  delete('kernel.mat')
end
save('kernel.mat', 'FrytB');
disp('Kernel completed');
