function [idx, box_coords] = pos2idx(x, nx, bl, min)
%pos2idx Returns the grid point index at a given position (x,x,x)

y = x;
z = x;

box_coords = zeros(3, 1);
box_coords(1) = floor((floor(x - min)) / bl);
box_coords(2) = floor((floor(y - min)) / bl);
box_coords(3) = floor((floor(z - min)) / bl);

fprintf("[%d, %d, %d]\n", box_coords(1), box_coords(2), box_coords(2));

% Plus 1 at the end because matlab indexes at 1
idx = box_coords(3) * nx * nx + box_coords(2) * nx + box_coords(1) + 1;

end

