%%s1载入采集文件并进行负数转换
clear all
DecArry = load('D:/prj/z1/src/CvCounter/line.txt');  
len = length(DecArry);
len_s = len;
sf = 1;
t = [0:len_s]/sf;
figure;
subplot(4,1,1);plot(DecArry);

%%s2 show abs
absArry = load('D:/prj/z1/src/CvCounter/2abs.txt');  
subplot(4,1,2);plot(absArry);

%%s3 show envelop
envlop = load('D:/prj/z1/src/CvCounter/3envelop.txt');  
subplot(4,1,3);plot(envlop);

%%s4 prydown 8
envlop = load('D:/prj/z1/src/CvCounter/4downFile.txt');  
subplot(4,1,4);plot(envlop);
xlabel('time(ms)');
title('vol');


% % title('Single-Sided Amplitude Spectrum of X(t)')
% % xlabel('f (Hz)')
% % ylabel('|P1(f)|')

% subplot(3,1,1);plot(s);
% title('Z1一次采集波形图');
%%2去除后面无效数据
% for i = nubers:length(DecArry)
%         DecArry(i) = 0;
% end

% plot(DecArry);
% X=[ DecArry(1952) DecArry(3904) DecArry(5856) ...
%     DecArry(7808) DecArry(9760) DecArry(11712)];

