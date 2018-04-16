function [ret, bl, bc] = DiffusionEuler(L, nx, D, T, M)
%DiffusionEuler Runs T timesteps of 3D diffusion based on Euler's method.
%The starting condition is a point source at position (0,0,0)
%   `L` defines the diffusion space ([-L, L] for all three dimensions)
%   `nx` is the number of gridpoint per dimension (nx*nx*nx) points in
%   total
%   `D` is the diffusion coefficient (i.e. determines speed of diffusion)
%   `T` is the total number of timesteps to run the diffusion
%   `M` is the initial 

% Cubic space, so these are all equal to nx
ny = nx;
nz = nx;

% The extents of the diffusion space
max = L;
min = -L;

c1 = zeros(nx*ny*nz, T);
% c2 = zeros(nx*ny*nz, T);

% This is a float (not integer)
bl = (max - min) / nx;

ibl2 = 1 / (bl*bl);
dt = 1;

% Instantaneous point source at center of space
fprintf('Source @ ');
[sidx, bc] = pos2idx(0, nx, bl, min);
c1(sidx, 1) = M / (bl^3);

for tt = 2:T
    for x = 1:nx
        for y = 1:ny
            for z = 1:nz
                c = (x-1) + (y-1)*nx + (z-1)*nx*ny;
                % index starts at 1
                c = c + 1;

                % Closed-edge boundary conditions
                if x == 1; e = c; else; e = c - 1; end
                if x == nx; w = c; else; w = c + 1; end
                if y == 1; n = c; else; n = c - nx; end
                if y == ny; s = c; else; s = c + nx; end
                if z == 1; b = c; else; b = c - nx*ny; end
                if z == nz; t = c; else; t = c + nx*ny; end

                % Save value for each timestep
                c1(c, tt) = c1(c, tt-1) + ...
                        D * dt * ibl2 * (c1(e, tt-1) - 2*c1(c, tt-1) + c1(w, tt-1)) + ...
                        D * dt * ibl2 * (c1(s, tt-1) - 2*c1(c, tt-1) + c1(n, tt-1)) + ...
                        D * dt * ibl2 * (c1(b, tt-1) - 2*c1(c, tt-1) + c1(t, tt-1));
             
%                 Do not save values for each timestep
%                 c2(c+1) = c1(c+1) + ...
%                         D * dt * ibl2 * (c1(e+1) - 2*c1(c+1) + c1(w+1)) + ...
%                         D * dt * ibl2 * (c1(s+1) - 2*c1(c+1) + c1(n+1)) + ...
%                         D * dt * ibl2 * (c1(b+1) - 2*c1(c+1) + c1(t+1));
                    
                % Leaking-edge boundary conditions
                if x == 1; c1(c, tt) = 0; end
                if x == nx; c1(c, tt) = 0; end
                if y == 1; c1(c, tt) = 0; end
                if y == ny; c1(c, tt) = 0; end
                if z == 1; c1(c, tt) = 0; end
                if z == nz; c1(c, tt) = 0; end

%                 tmp = c1;
%                 c1 = c2;
%                 c2 = tmp;
            end
        end
    end
end

% Return all the concentration values for all time steps
ret = c1;
end
