function threads_data = num_thread_plot(folderName)

files = dir([folderName '/' '*.txt']);
filenames = {files.name};

for i = 1:length(filenames)
    filename = filenames(i);
    pathname = strcat(folderName, '/', filename{1});
    
    threads_data{1,i} = filename{1};
    threads_data{2,i} = csvread(pathname);
        
    [~, ~, ii ] = unique(threads_data{2,1}(:,5));
    for j = 2:(length(threads_data{2,1}(1,:)) - 1)
        threads_data{3,i}(:,j-1) = accumarray(ii, threads_data{2,i}(:,j), [], @mean)';
    end
    
    % make array for the stacked bar
    for k = 1:3
        threads_data{4,i}(:,k) = threads_data{3,i}(end,k);
    end
    
    % subtract search and build time from compute time for stacked bar
    threads_data{4,i}(:,end) = threads_data{4,i}(:,end) - threads_data{4,i}(:,1) - threads_data{4,i}(:,2);
end

figure()
hold on

cells_per_dim = unique(threads_data{2,1}(:,5));
for i = 1:length(threads_data)
    % compute times
    plot(cells_per_dim, threads_data{3,i}(:,end))
end

legend(threads_data(1,:))

figure()
for j = 1:4
    for i = 1:length(threads_data{4})
        y(j, i) = threads_data{4,j}(i);
    end
end

bar(y, 'stacked')
legend('build time', 'search time', 'remaining');
set(gca,'xticklabel',threads_data(1,:))

end