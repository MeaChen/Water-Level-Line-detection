function result = median_filter(pic,n)
    [height, width]=size(pic);   %ÊäÈëÍ¼ÏñÊÇp¡ÁqµÄ,ÇÒp>n,q>n  
    result=pic; 
    for i=1:height-n+1  
        for j=1:width-n+1  
            temp = median(pic(i:i+n-1,j:j+n-1));
            result(i+(n-1)/2,j+(n-1)/2) = median(temp);  
        end  
    end    
end