function U = recon(A, M, arravg, arr)
  gridXres = 0.04;
  gridYres = gridXres;
  RoomLength = 2;
  RoomWidth = RoomLength;
  tx = [gridXres/2:gridXres:RoomLength-gridXres/2]-RoomLength/2;
  ty = [RoomWidth-gridYres/2:-gridYres:gridYres/2]-RoomWidth/2;
  arr = arr(:);
  arravg = arravg(:);
  arrult = (arr - arravg)./(20*log10(exp(1)));
  c = 3e8; freq = 2.4e9; lambda = c/freq;
  opts.mu = 4;
  opts.beta = 16;
  opts.TVnorm = 1;
  opts.maxit =5000;
  opts.tol = 1E-10;
  opts.nonneg = true;
  lambY = 1;
  lambX = 1;
  O = TVAL3(A,arrult,M,2*M,opts,lambX,lambY);
  Oreal = O(1:M,1:M);
  Oimag = O(1:M,M+1:2*M);

  Atten = 4*pi*(Oimag*0.5)./lambda; % Estimate Attenuation
  %% Reconstruction
  Atten(Atten<0) = 0; % Remove values below zero (this is optional)
  figure(2)
  imagesc(tx,ty,Atten); hold on;
  colormap jet
  view([0 -90])
  colorbar
  ax = gca();
  set(gca, 'fontsize', 18);
  title('xPRA Reconstruction')
  hold off;
  U = 5;
