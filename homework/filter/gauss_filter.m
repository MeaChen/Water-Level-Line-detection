function result = gauss_filter(pic,n,k)
    a = fspecial('gaussian',n,k);
    result=pic;
    [height,width]=size(pic);
    x=double(pic);
    for i=1:height-n+1
        for j=1:width-n+1
            c=x(i:i+n-1,j:j+n-1).*a;  %µã³Ë
            result(i+(n-1)/2,j+(n-1)/2)=sum(sum(c));  %ÇóºÍ
        end
    end
end