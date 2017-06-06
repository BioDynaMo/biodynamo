function data = num_cell_plot(folderName)

files = dir([folderName '/' '*.txt']);
filenames = {files.name};

for i = 1:length(filenames)
    filename = filenames(i);
    pathname = strcat(folderName, '/', filename{1});
    
    data{1,i} = filename{1};
    data{2,i} = csvread(pathname);
    
    [~, ~, ii ] = unique(data{2,1}(:,1));
    for j = 2:(length(data{2,1}(1,:)) - 1)
        data{3,i}(:,j-1) = accumarray(ii, data{2,i}(:,j), [], @mean)';
    end
    
    % make array for the stacked bar
    for k = 1:3
        data{4,i}(:,k) = data{3,i}(end,k);
    end
    
    % subtract search and build time from compute time for stacked bar
    data{4,i}(:,end) = data{4,i}(:,end) - data{4,i}(:,1) - data{4,i}(:,2);
end

figure()
hold on

cells_per_dim = unique(data{2,1}(:,1));
for i = 1:length(data)
    % compute times
    plot(cells_per_dim, data{3,i}(:,(end-1)))
end

legend(data(1,:))

figure()
for j = 1:4
    for i = 1:length(data{4})
        y(j, i) = data{4,j}(i);
    end
end
bar(y, 'stacked')
legend('build time', 'search time', 'remaining');
set(gca,'xticklabel',data(1,:))
end