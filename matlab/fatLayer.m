clear all
close all
img = imread('fat_layer.bmp');
%figure,imshow(img);
img = rgb2gray(img);
BW = edge(img,'Canny',[],10); figure,imshow(BW);
% BW = edge(img,'canny');
img_gradient = gradient(double(img));
CC = bwconncomp(BW);               
%coordinates of the lines
Plist_candi = struct2cell(regionprops(CC,'PixelList'));  
figure,imshow(BW);
%position of the points of a line in the image
line_candi = CC.PixelIdxList;
numPixels = cellfun(@numel,line_candi);
length_pixel =1;
w1=2;
w2=3;
w3 =4;
cost = ones(1,size(line_candi,2));
for i=1:size(line_candi,2)
    edge_current = Plist_candi{i};
    %how many pixels does the current edge have?
    length_pixel = length(line_candi{i})/length_pixel;
    %comparison of length in horizontal to pixel number, this value is
    %better when it is close to 1;
    length_hiro = max(edge_current(:,1))-min(edge_current(:,1))/length_pixel;
    %depth of the edge
    edge_depth = sum(edge_current,2)/length_pixel;
    %gradience of the edge
    edge_gradient = sum(img_gradient(line_candi{i}))/length_pixel;
    %cost function
    %cost(i) = length_hiro*w1+edge_depth*w2+edge_gradient*w3;
    j = length_hiro*w1+edge_depth*w2+edge_gradient*w3;

    %for test
%     figure;
%     hold on;
%     imagesc(BW);
%     plot(line_candi{i});
%     hold off

end

        BW(line_candi{i}) = 0;

figure,imshow(BW)
% to remove lines who is not long horizentally
