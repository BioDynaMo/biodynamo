% Simulate diffusion in 3D space starting with a single point source.
% It is possible to run with different resolutions.

M = 1e5;    % initial point source concentration
D = 0.5;    % diffusion coefficient
pos = 10;   % to be measured position value (= marker)
x = pos;
y = pos;
z = pos;
T = 1000;   % number of diffusion iterations
syms t;
figure()

% Subplot index
i = 1;

% The range of resolutions to plot for
R = 10:10:90;

% n x n subplots
n = ceil(sqrt(length(R)));

for res = R
    % The number of gridpoints per dimension
    nx = res;

    % Length of the diffusion space's dimensions (L*L*L)
    L = 100;

    % Run numerical diffusion for T timesteps
    [conc, bl, bc1] = DiffusionEuler(L, nx, D, T, M);
    
    fprintf("Box length = %.4f\n", bl);
    fprintf("Plotting values @ ");

    % Get index of the marker
    [marker_idx, bc2] = pos2idx(pos, nx, bl, -L);
    subplot(n, n, i)
    plot(conc(marker_idx, :))
    title(sprintf('Resolution = %d', res));
    
    hold on
    
    % The distance between the point source and the marker
    dist = norm(bl.*(bc2 - bc1));
    x = bl*(bc2(1) - bc1(1));
    y = bl*(bc2(2) - bc1(2));
    z = bl*(bc2(3) - bc1(3));
    fprintf("dist = %.4f\n", dist);
    fprintf("norm([x y z]) = %.4f\n", norm([x y z]));
    
    % Analytical solution plot
    fplot((M/power(4*pi*D*t, 3/2))*exp(-(x^2)/(4*D*t) - (y^2)/(4*D*t) - (z^2)/(4*D*t)), [1 T]);

    fprintf("Marker @ %.4f from the point source (step = %.4f)\n", dist, norm([bl bl bl]));
    fprintf("Error = %.4f\n\n", abs(dist - norm([x y z])));

    legend('numerical', 'analytical')
    i = i + 1;
end
