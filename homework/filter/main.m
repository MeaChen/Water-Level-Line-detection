function main
    pic = imread('filters.png');
    n=input('the height of the filter operator:');
    result1=mean_filter(pic,n);
    result2=gauss_filter(pic,n,1);
    result3=median_filter(pic,n);
    figure(1);
    imshow(result1);
    figure(2);
    imshow(result2);
    figure(3);
    imshow(result3);
    
end