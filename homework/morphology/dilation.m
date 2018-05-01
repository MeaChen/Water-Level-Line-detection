function result = dilation(src,se)
    result = src;
    [height,width] = size(src);
    [se1,se2] = size(se);
    if se1>height || se2>width
        error '结构元素长或宽超过目标图片';
    end
    if mod(se1,2)==0 || mod(se2,2)==0
        error '结构元素长或宽不可为偶数';
    end
    offset1 = floor(se1/2);
    offset2 = floor(se2/2);
    for i = (1+offset1):(height-offset1)  %扫描图片每个像素（除边界）
        for j=(1+offset2):(width-offset2)
            flag = 1;
            for x=(i-offset1):(i+offset1)
                for y=(j-offset2):(j+offset2)
                    if(se(x-i+offset1+1,y-j+offset2+1))
                        if src(x,y)==1
                            result(i,j)=1;
                            flag=0;
                            break;
                        end
                    end
                end
            end
            if(flag==1)
                result(x,y)=0;
            end
        end
    end
end