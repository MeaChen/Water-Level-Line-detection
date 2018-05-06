% 实现erosion，dilation的形态学操作
% 说明：自定义输入文件名，读取图片像素作为结构元素
% 说明：选择图片中心点作为锚点，因此不支持图片像素矩阵的长、宽为偶数，也不支持长、宽大于目标图片的长、宽
% 说明：结果为输出两张经过膨胀、腐蚀操作的图片

function main
    %White = 0, black = 1
    pic = imread('lena-binary.bmp');
    s=input('请输入图片名称：','s');
    SE = imread(s);
    
    [height,width]=size(pic);
    re1 = dilation(pic,SE);
    imwrite(re1,'dilation.bmp','bmp');
    re2 = erosion(pic,SE);
    imwrite(re2,'erosion.bmp','bmp');

end
